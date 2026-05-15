#include "spline_visualization.h"

#include "array_sequence.h"
#include "cubic_spline.h"
#include "dynamic_array.h"
#include "linear_spline.h"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#endif

// Убираем GLFW_INCLUDE_NONE, чтобы GLFW сам включил нужный OpenGL заголовок
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

namespace {

constexpr int kPlotSamples = 500; // на сколько дискретных точек разбивается функция для отрисовки
constexpr double kSineMinX = -1.0;
constexpr double kSineMaxX = 1.0;
constexpr double kSineNodeStep = 0.5;
constexpr double kSineFrequency = 4.0;
constexpr double kSineZoomMinX = -1.1;
constexpr double kSineZoomMaxX = 1.1;
constexpr double kSineZoomMinY = -1.5;
constexpr double kSineZoomMaxY = 1.5;

// данные графика
struct PlotData {
    DynamicArray<double> node_x;
    DynamicArray<double> node_y;
    DynamicArray<double> spline_x;
    DynamicArray<double> spline_y;
    DynamicArray<double> reference_x;
    DynamicArray<double> reference_y;
};

enum class ViewerMode {
    Points,
    Sine,
};

struct InterpolatorOption {
    const char *name;
    Interpolator<double> *interpolator;
};

double reference_sine(double x) { return std::sin(kSineFrequency * x); }

void destroy_segments(Sequence<Function<double> *> *&segments) {
    if (segments == nullptr) {
        return;
    }

    for (int index = 0; index < segments->get_count(); ++index) {
        delete segments->get(index);
    }
    delete segments;
    segments = nullptr;
}

void clear_plot_data(PlotData &data) {
    data.node_x.resize(0);
    data.node_y.resize(0);
    data.spline_x.resize(0);
    data.spline_y.resize(0);
    data.reference_x.resize(0);
    data.reference_y.resize(0);
}

void collect_nodes(const Sequence<Point<double>> &points, PlotData &data) {
    const int count = points.get_count();
    data.node_x.resize(count);
    data.node_y.resize(count);

    for (int index = 0; index < count; ++index) {
        const Point<double> &point = points.get(index);
        data.node_x.set(index, point.x);
        data.node_y.set(index, point.y);
    }
}

bool is_valid_append(const Sequence<Point<double>> &points, const Point<double> &point,
                     std::string &message) {
    if (points.get_count() > 0 && point.x <= points.get_last().x) {
        message = "X for end insertion must be greater than the last point X";
        return false;
    }
    return true;
}

bool is_valid_prepend(const Sequence<Point<double>> &points, const Point<double> &point,
                      std::string &message) {
    if (points.get_count() > 0 && point.x >= points.get_first().x) {
        message = "X for beginning insertion must be less than the first point X";
        return false;
    }
    return true;
}

bool parse_double_value(const char *text, const char *field_name, double &value,
                        std::string &message) {
    errno = 0;
    char *end = nullptr;
    const double parsed = std::strtod(text, &end);

    if (text == end || errno == ERANGE || !std::isfinite(parsed)) {
        message = std::string(field_name) + " must be a valid number";
        return false;
    }

    while (*end == ' ' || *end == '\t') {
        ++end;
    }

    if (*end != '\0') {
        message = std::string(field_name) + " must contain only a number";
        return false;
    }

    value = parsed;
    return true;
}

bool parse_point_input(const char *x_text, const char *y_text, Point<double> &point,
                       std::string &message) {
    return parse_double_value(x_text, "X", point.x, message) &&
           parse_double_value(y_text, "Y", point.y, message);
}

bool rebuild_spline(const Sequence<Point<double>> &points, Interpolator<double> &interpolator,
                    Sequence<Function<double> *> *&segments, std::string &message) {
    try {
        Sequence<Function<double> *> *new_segments = interpolator.interpolate(points);
        destroy_segments(segments);
        segments = new_segments;
    } catch (const std::exception &error) {
        destroy_segments(segments);
        message = error.what();
        return false;
    }

    message = "Interpolation rebuilt";
    return true;
}

bool refresh_plot(const Sequence<Point<double>> &points, Interpolator<double> &interpolator,
                  const Sequence<Function<double> *> *segments, PlotData &data,
                  std::string &message) {
    collect_nodes(points, data);
    data.spline_x.resize(0);
    data.spline_y.resize(0);

    if (segments == nullptr) {
        return false;
    }

    const Point<double> &first = points.get_first();
    const Point<double> &last = points.get_last();
    if (first.x >= last.x) {
        message = "Cannot plot a spline with an invalid X range";
        return false;
    }

    data.spline_x.resize(kPlotSamples);
    data.spline_y.resize(kPlotSamples);
    for (int index = 0; index < kPlotSamples; ++index) {
        const double t = static_cast<double>(index) / static_cast<double>(kPlotSamples - 1);
        const double x = first.x + (last.x - first.x) * t;
        const Option<double> value = interpolator.evaluate(*segments, x);
        if (!value.has_value()) {
            message = "Cannot evaluate spline inside the plotting range";
            data.spline_x.resize(0);
            data.spline_y.resize(0);
            return false;
        }
        data.spline_x.set(index, x);
        data.spline_y.set(index, value.get_value());
    }

    return true;
}

bool rebuild_and_refresh(const Sequence<Point<double>> &points, Interpolator<double> &interpolator,
                         Sequence<Function<double> *> *&segments, PlotData &data,
                         std::string &message) {
    bool spline_ready = rebuild_spline(points, interpolator, segments, message);
    if (spline_ready) {
        spline_ready = refresh_plot(points, interpolator, segments, data, message);
    } else {
        refresh_plot(points, interpolator, segments, data, message);
    }
    return spline_ready;
}

void fill_sine_nodes(MutableArraySequence<Point<double>> &points, double min_x, double max_x,
                     double step) {
    if (step <= 0.0 || min_x >= max_x) {
        return;
    }

    for (double x = min_x; x <= max_x + step * 0.5; x += step) {
        points.append(Point<double>{x, reference_sine(x)});
    }
}

void fill_sine_reference(PlotData &data, double min_x, double max_x) {
    data.reference_x.resize(kPlotSamples);
    data.reference_y.resize(kPlotSamples);

    for (int index = 0; index < kPlotSamples; ++index) {
        const double t = static_cast<double>(index) / static_cast<double>(kPlotSamples - 1);
        const double x = min_x + (max_x - min_x) * t;
        data.reference_x.set(index, x);
        data.reference_y.set(index, reference_sine(x));
    }
}

void initialize_sine_state(MutableArraySequence<Point<double>> *&points,
                           Sequence<Function<double> *> *&segments,
                           Interpolator<double> &interpolator, PlotData &data, bool &spline_ready,
                           std::string &message) {
    if (points != nullptr) {
        delete points;
    }
    points = new MutableArraySequence<Point<double>>();
    destroy_segments(segments);
    segments = nullptr;
    clear_plot_data(data);

    fill_sine_nodes(*points, kSineMinX, kSineMaxX, kSineNodeStep);
    if (points->get_count() >= 2) {
        spline_ready = rebuild_and_refresh(*points, interpolator, segments, data, message);
    } else {
        message = "Not enough points to build sine interpolation";
        spline_ready = false;
    }
    fill_sine_reference(data, kSineMinX, kSineMaxX);
}

bool add_point(MutableArraySequence<Point<double>> &points, const Point<double> &point,
               bool to_beginning, Interpolator<double> &interpolator,
               Sequence<Function<double> *> *&segments, std::string &message) {
    if (to_beginning) {
        if (!is_valid_prepend(points, point, message)) {
            return false;
        }
    } else if (!is_valid_append(points, point, message)) {
        return false;
    }

    try {
        if (to_beginning) {
            points.prepend(point);
            message = "Point added to the beginning";
        } else {
            points.append(point);
            message = "Point added to the end";
        }
    } catch (const std::exception &error) {
        message = error.what();
        return false;
    }

    if (points.get_count() >= 2) {
        rebuild_spline(points, interpolator, segments, message);
    }
    return true;
}

bool add_point_sorted(MutableArraySequence<Point<double>> &points, const Point<double> &point,
                      Interpolator<double> &interpolator, Sequence<Function<double> *> *&segments,
                      std::string &message) {
    int insert_index = 0;
    while (insert_index < points.get_count() && points.get(insert_index).x < point.x) {
        ++insert_index;
    }

    if (insert_index < points.get_count() && points.get(insert_index).x == point.x) {
        message = "Point X values must be unique";
        return false;
    }

    try {
        points.insert_at(point, insert_index);
        message = "Point added from plot";
    } catch (const std::exception &error) {
        message = error.what();
        return false;
    }

    if (points.get_count() >= 2) {
        rebuild_spline(points, interpolator, segments, message);
    }
    return true;
}

// таблица точек
void draw_points_table(const PlotData &data) {
    if (!ImGui::BeginTable("PointsTable", 3,
                           ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                               ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("#");
    ImGui::TableSetupColumn("x");
    ImGui::TableSetupColumn("y");
    ImGui::TableHeadersRow();

    for (int index = 0; index < data.node_x.get_size(); ++index) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", index);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.6g", data.node_x.get(index));
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.6g", data.node_y.get(index));
    }

    ImGui::EndTable();
}

std::string format_segment(const Function<double> *segment) {
    std::string result;
    char buffer[160];
    if (segment == nullptr) {
        return "<null>";
    }

    const PolynomialFunction<double> *poly =
        dynamic_cast<const PolynomialFunction<double> *>(segment);
    if (poly == nullptr) {
        std::snprintf(buffer, sizeof(buffer), "f(x), %.6g <= x <= %.6g", segment->get_left(),
                      segment->get_right());
        return std::string(buffer);
    }

    const DynamicArray<double> &coefficients = poly->get_coefficients();
    const int coefficient_count = coefficients.get_size();

    if (coefficient_count == 0) {
        result = "0";
    }

    for (int index = 0; index < coefficient_count; ++index) {
        const double coefficient = coefficients.get(index);
        if (index == 0) {
            std::snprintf(buffer, sizeof(buffer), "%.6g", coefficient);
        } else if (index == 1) {
            std::snprintf(buffer, sizeof(buffer), " + %.6g(x - %.6g)", coefficient,
                          poly->get_left());
        } else {
            std::snprintf(buffer, sizeof(buffer), " + %.6g(x - %.6g)^%d", coefficient,
                          poly->get_left(), index);
        }
        result += buffer;
    }

    std::snprintf(buffer, sizeof(buffer), ", %.6g <= x <= %.6g", poly->get_left(),
                  poly->get_right());
    result += buffer;
    return result;
}

// пишем уравнение кусочно заданной функции - кубического сплайна в аналитическом виде
void draw_piecewise_system(const Sequence<Function<double> *> *segments, bool spline_ready) {
    ImGui::TextUnformatted("Piecewise form");
    ImGui::Separator();

    if (!spline_ready || segments == nullptr) {
        ImGui::TextWrapped("Build interpolation from enough points to see polynomial segments.");
        return;
    }

    ImGui::TextUnformatted("S(x) =");
    ImGui::Indent();
    for (int index = 0; index < segments->get_count(); ++index) {
        const std::string formula = format_segment(segments->get(index));
        ImGui::TextWrapped("%s", formula.c_str());
    }
    ImGui::Unindent();
}

void run_spline_viewer(InterpolatorOption *interpolators, int interpolator_count) {
    std::cout << "\n=== Spline Viewer ===" << std::endl;
    std::cout << "Launching graphical window..." << std::endl;

    if (interpolators == nullptr || interpolator_count <= 0 ||
        interpolators[0].interpolator == nullptr) {
        std::cout << "No interpolation algorithms configured" << std::endl;
        return;
    }

    PlotData points_data;
    std::string points_status = "Create a point sequence";
    PlotData sine_data;
    std::string sine_status = "Comparison interpolation ready";
    char input_x[64] = "0";
    char input_y[64] = "0";
    bool points_ready = false;
    bool sine_ready = false;
    int current_interpolator_index = 0;
    int points_interpolator_index = 0;
    int sine_interpolator_index = 0;
    ViewerMode mode = ViewerMode::Points;

    // Инициализация GLFW + ImGui + ImPlot
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW" << std::endl;
        return;
    }

    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // На macOS обязательно
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window =
        glfwCreateWindow(1280, 720, "Cubic Spline Interpolation", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cout << "Failed to create window" << std::endl;
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    MutableArraySequence<Point<double>> *points = new MutableArraySequence<Point<double>>();
    Sequence<Function<double> *> *segments = nullptr;
    MutableArraySequence<Point<double>> *sine_points = new MutableArraySequence<Point<double>>();
    Sequence<Function<double> *> *sine_segments = nullptr;
    initialize_sine_state(sine_points, sine_segments,
                          *interpolators[current_interpolator_index].interpolator, sine_data,
                          sine_ready, sine_status);

    // Главный цикл окна
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        ImGui::Begin("Cubic Spline Interpolation", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        ImGui::BeginChild("Controls", ImVec2(330.0f, 0.0f), true);
        ImGui::Text("Spline points");
        ImGui::Separator();

        ImGui::Text("Mode");
        bool is_points_mode = mode == ViewerMode::Points;
        bool is_sine_mode = mode == ViewerMode::Sine;
        if (ImGui::RadioButton("Points", is_points_mode)) {
            mode = ViewerMode::Points;
            if (points_interpolator_index != current_interpolator_index &&
                points->get_count() >= 2) {
                points_ready = rebuild_and_refresh(
                    *points, *interpolators[current_interpolator_index].interpolator, segments,
                    points_data, points_status);
                points_interpolator_index = current_interpolator_index;
            }
        }
        if (ImGui::RadioButton("Comparison", is_sine_mode)) {
            mode = ViewerMode::Sine;
            if (sine_interpolator_index != current_interpolator_index) {
                sine_ready = rebuild_and_refresh(
                    *sine_points, *interpolators[current_interpolator_index].interpolator,
                    sine_segments, sine_data, sine_status);
                fill_sine_reference(sine_data, kSineMinX, kSineMaxX);
                sine_interpolator_index = current_interpolator_index;
            }
        }
        ImGui::Separator();

        ImGui::Text("Algorithm");
        for (int index = 0; index < interpolator_count; ++index) {
            if (interpolators[index].interpolator == nullptr) {
                continue;
            }
            if (ImGui::Button(interpolators[index].name, ImVec2(-1.0f, 0.0f))) {
                current_interpolator_index = index;
                if (mode == ViewerMode::Points) {
                    if (points->get_count() >= 2) {
                        points_ready =
                            rebuild_and_refresh(*points, *interpolators[index].interpolator,
                                                segments, points_data, points_status);
                    } else {
                        destroy_segments(segments);
                        points_data.spline_x.resize(0);
                        points_data.spline_y.resize(0);
                        points_status = std::string(interpolators[index].name) + " selected";
                        points_ready = false;
                    }
                    points_interpolator_index = index;
                } else {
                    sine_ready =
                        rebuild_and_refresh(*sine_points, *interpolators[index].interpolator,
                                            sine_segments, sine_data, sine_status);
                    fill_sine_reference(sine_data, kSineMinX, kSineMaxX);
                    sine_interpolator_index = index;
                }
            }
        }
        ImGui::Separator();

        if (mode == ViewerMode::Points) {
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText("x", input_x, sizeof(input_x));
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText("y", input_y, sizeof(input_y));

            if (ImGui::Button("Add begin", ImVec2(-1.0f, 0.0f))) {
                Point<double> point{};
                if (parse_point_input(input_x, input_y, point, points_status) &&
                    add_point(*points, point, true,
                              *interpolators[current_interpolator_index].interpolator, segments,
                              points_status)) {
                    points_ready = refresh_plot(
                        *points, *interpolators[current_interpolator_index].interpolator, segments,
                        points_data, points_status);
                }
            }
            if (ImGui::Button("Add end", ImVec2(-1.0f, 0.0f))) {
                Point<double> point{};
                if (parse_point_input(input_x, input_y, point, points_status) &&
                    add_point(*points, point, false,
                              *interpolators[current_interpolator_index].interpolator, segments,
                              points_status)) {
                    points_ready = refresh_plot(
                        *points, *interpolators[current_interpolator_index].interpolator, segments,
                        points_data, points_status);
                }
            }
            if (ImGui::Button("Rebuild interpolation", ImVec2(-1.0f, 0.0f))) {
                points_ready = rebuild_and_refresh(
                    *points, *interpolators[current_interpolator_index].interpolator, segments,
                    points_data, points_status);
            }
            if (ImGui::Button("Clear", ImVec2(-1.0f, 0.0f))) {
                delete points;
                destroy_segments(segments);
                points = new MutableArraySequence<Point<double>>();
                segments = nullptr;
                clear_plot_data(points_data);
                points_ready = false;
                points_status = "Create a point sequence";
            }
        } else {
            ImGui::Text("Sine range: [%.3f, %.3f]", kSineMinX, kSineMaxX);
            ImGui::Text("Function: sin(%.6gx)", kSineFrequency);
            ImGui::Text("Node step: %.3f", kSineNodeStep);
        }

        ImGui::Separator();
        std::string &active_status = (mode == ViewerMode::Points) ? points_status : sine_status;
        MutableArraySequence<Point<double>> *active_points =
            (mode == ViewerMode::Points) ? points : sine_points;
        PlotData &active_data = (mode == ViewerMode::Points) ? points_data : sine_data;
        bool &active_ready = (mode == ViewerMode::Points) ? points_ready : sine_ready;

        ImGui::TextWrapped("%s", active_status.c_str());
        ImGui::Text("Algorithm: %s", interpolators[current_interpolator_index].name);
        ImGui::Text("Points: %d", active_points->get_count());
        draw_points_table(active_data);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("PlotAndFormula", ImVec2(0.0f, 0.0f), false);
        float plot_height = ImGui::GetContentRegionAvail().y * 0.58f;
        if (plot_height < 220.0f) {
            plot_height = 220.0f;
        }

        if (mode == ViewerMode::Points) {
            if (ImPlot::BeginPlot("Spline interpolation", ImVec2(-1.0f, plot_height))) {
                ImPlot::SetupAxes("x", "y");
                if (ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    const ImPlotPoint mouse_position = ImPlot::GetPlotMousePos();
                    const Point<double> point{mouse_position.x, mouse_position.y};
                    if (add_point_sorted(*points, point,
                                         *interpolators[current_interpolator_index].interpolator,
                                         segments, points_status)) {
                        points_ready = refresh_plot(
                            *points, *interpolators[current_interpolator_index].interpolator,
                            segments, points_data, points_status);
                    }
                }

                if (points_ready && points_data.spline_x.get_size() > 0) {
                    ImPlot::PlotLine("Interpolation", &points_data.spline_x.get(0),
                                     &points_data.spline_y.get(0), points_data.spline_x.get_size());
                }
                if (points_data.node_x.get_size() > 0) {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotScatter("Nodes", &points_data.node_x.get(0),
                                        &points_data.node_y.get(0), points_data.node_x.get_size());
                }
                ImPlot::EndPlot();
            }
        } else {
            if (ImPlot::BeginPlot("sin(4x) and interpolation", ImVec2(-1.0f, plot_height))) {
                ImPlot::SetupAxes("x", "y");
                ImPlot::SetupAxesLimits(kSineZoomMinX, kSineZoomMaxX, kSineZoomMinY, kSineZoomMaxY,
                                        ImPlotCond_Once);
                if (sine_ready &&
                    sine_data.reference_x.get_size() == sine_data.spline_y.get_size() &&
                    sine_data.reference_x.get_size() > 0) {
                    ImPlot::SetNextFillStyle(ImVec4(0.95f, 0.45f, 0.1f, 1.0f), 0.22f);
                    ImPlot::PlotShaded("Difference", &sine_data.reference_x.get(0),
                                       &sine_data.reference_y.get(0), &sine_data.spline_y.get(0),
                                       sine_data.reference_x.get_size());
                }
                if (sine_data.reference_x.get_size() > 0) {
                    ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.7f, 0.25f, 1.0f), 1.6f);
                    ImPlot::PlotLine("sin(4x)", &sine_data.reference_x.get(0),
                                     &sine_data.reference_y.get(0),
                                     sine_data.reference_x.get_size());
                }
                if (sine_ready && sine_data.spline_x.get_size() > 0) {
                    ImPlot::SetNextLineStyle(ImVec4(0.95f, 0.45f, 0.1f, 1.0f), 1.9f);
                    if (std::string(interpolators[current_interpolator_index].name) == "Linear" &&
                        sine_data.node_x.get_size() > 0) {
                        ImPlot::PlotLine("Interpolation", &sine_data.node_x.get(0),
                                         &sine_data.node_y.get(0), sine_data.node_x.get_size());
                    } else {
                        ImPlot::PlotLine("Interpolation", &sine_data.spline_x.get(0),
                                         &sine_data.spline_y.get(0), sine_data.spline_x.get_size());
                    }
                }
                if (sine_data.node_x.get_size() > 0) {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0f,
                                               ImVec4(0.1f, 0.2f, 0.9f, 1.0f), 1.5f,
                                               ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImPlot::PlotScatter("Nodes", &sine_data.node_x.get(0), &sine_data.node_y.get(0),
                                        sine_data.node_x.get_size());
                }
                ImPlot::EndPlot();
            }
        }

        ImGui::Spacing();
        ImGui::BeginChild("PiecewiseSystem", ImVec2(0.0f, 0.0f), true);
        Sequence<Function<double> *> *active_segments =
            (mode == ViewerMode::Points) ? segments : sine_segments;
        draw_piecewise_system(active_segments, active_ready);
        ImGui::EndChild();
        ImGui::EndChild();
        ImGui::End();

        // Рендеринг
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Очистка
    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    delete points;
    destroy_segments(segments);
    delete sine_points;
    destroy_segments(sine_segments);
    std::cout << "Graphical window closed. Returning to menu." << std::endl;
}

} // namespace

void menu_spline_viewer() {
    CubicSplineInterpolator<double> cubic_interpolator;
    LinearSplineInterpolator<double> linear_interpolator;
    InterpolatorOption interpolators[] = {
        {"Cubic", &cubic_interpolator},
        {"Linear", &linear_interpolator},
    };
    run_spline_viewer(interpolators, 2);
}
