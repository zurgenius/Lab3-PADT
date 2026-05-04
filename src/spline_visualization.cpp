#include "spline_visualization.h"

#include "cubic_spline.h"
#include "dynamic_array.h"

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

// Но на macOS нужен дополнительный флаг для подавления предупреждений об устаревании OpenGL
#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

namespace {

constexpr int kMinSplinePoints = 3; // сколько точек минимально должно быть введено юзером
constexpr int kPlotSamples = 500;   // на сколько дискретных точек разбивается функция для отрисовки

// данные графика
struct PlotData {
    DynamicArray<double> node_x;
    DynamicArray<double> node_y;
    DynamicArray<double> spline_x;
    DynamicArray<double> spline_y;
};

void clear_plot_data(PlotData &data) {
    data.node_x.resize(0);
    data.node_y.resize(0);
    data.spline_x.resize(0);
    data.spline_y.resize(0);
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
                    Sequence<FunctionSegment<double>> *&segments, std::string &message) {
    if (points.get_count() < kMinSplinePoints) {
        message = "Add at least three points before building the spline";
        return false;
    }

    try {
        Sequence<FunctionSegment<double>> *new_segments = interpolator.interpolate(points);
        delete segments;
        segments = new_segments;
    } catch (const std::exception &error) {
        message = error.what();
        return false;
    }

    message = "Spline rebuilt";
    return true;
}

bool refresh_plot(const Sequence<Point<double>> &points, Interpolator<double> &interpolator,
                  const Sequence<FunctionSegment<double>> *segments, PlotData &data,
                  std::string &message) {
    collect_nodes(points, data);
    data.spline_x.resize(0);
    data.spline_y.resize(0);

    if (points.get_count() < kMinSplinePoints || segments == nullptr) {
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

bool add_point(MutableArraySequence<Point<double>> &points, const Point<double> &point,
               bool to_beginning, Interpolator<double> &interpolator,
               Sequence<FunctionSegment<double>> *&segments, std::string &message) {
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

    if (points.get_count() >= kMinSplinePoints) {
        return rebuild_spline(points, interpolator, segments, message);
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

std::string format_segment(const FunctionSegment<double> &segment) {
    std::string result;
    char buffer[160];
    const int coefficient_count = segment.coefficients.get_size();

    if (coefficient_count == 0) {
        result = "0";
    }

    for (int index = 0; index < coefficient_count; ++index) {
        const double coefficient = segment.coefficients.get(index);
        if (index == 0) {
            std::snprintf(buffer, sizeof(buffer), "%.6g", coefficient);
        } else if (index == 1) {
            std::snprintf(buffer, sizeof(buffer), " + %.6g(x - %.6g)", coefficient, segment.left);
        } else {
            std::snprintf(buffer, sizeof(buffer), " + %.6g(x - %.6g)^%d", coefficient, segment.left,
                          index);
        }
        result += buffer;
    }

    std::snprintf(buffer, sizeof(buffer), ", %.6g <= x <= %.6g", segment.left, segment.right);
    result += buffer;
    return result;
}

// пишем уравнение кусочно заданной функции - кубического сплайна в аналитическом виде
void draw_piecewise_system(const Sequence<FunctionSegment<double>> *segments, bool spline_ready) {
    ImGui::TextUnformatted("Piecewise form");
    ImGui::Separator();

    if (!spline_ready || segments == nullptr) {
        ImGui::TextWrapped("Build a spline from at least three points to see polynomial segments.");
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

} // namespace

void menu_spline_viewer(Interpolator<double> &interpolator) {
    std::cout << "\n=== Spline Viewer ===" << std::endl;
    std::cout << "Launching graphical window..." << std::endl;

    PlotData data;
    std::string status = "Create a point sequence";
    char input_x[64] = "0";
    char input_y[64] = "0";
    bool spline_ready = false;

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
    Sequence<FunctionSegment<double>> *segments = nullptr;

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

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("x", input_x, sizeof(input_x));
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("y", input_y, sizeof(input_y));

        if (ImGui::Button("Add begin", ImVec2(-1.0f, 0.0f))) {
            Point<double> point{};
            if (parse_point_input(input_x, input_y, point, status) &&
                add_point(*points, point, true, interpolator, segments, status)) {
                spline_ready = refresh_plot(*points, interpolator, segments, data, status);
            }
        }
        if (ImGui::Button("Add end", ImVec2(-1.0f, 0.0f))) {
            Point<double> point{};
            if (parse_point_input(input_x, input_y, point, status) &&
                add_point(*points, point, false, interpolator, segments, status)) {
                spline_ready = refresh_plot(*points, interpolator, segments, data, status);
            }
        }
        if (ImGui::Button("Rebuild spline", ImVec2(-1.0f, 0.0f))) {
            spline_ready = rebuild_spline(*points, interpolator, segments, status);
            if (spline_ready) {
                spline_ready = refresh_plot(*points, interpolator, segments, data, status);
            }
        }
        if (ImGui::Button("Clear", ImVec2(-1.0f, 0.0f))) {
            delete points;
            delete segments;
            points = new MutableArraySequence<Point<double>>();
            segments = nullptr;
            clear_plot_data(data);
            spline_ready = false;
            status = "Create a point sequence";
        }

        ImGui::Separator();
        ImGui::TextWrapped("%s", status.c_str());
        ImGui::Text("Points: %d", points->get_count());
        draw_points_table(data);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("PlotAndFormula", ImVec2(0.0f, 0.0f), false);
        float plot_height = ImGui::GetContentRegionAvail().y * 0.58f;
        if (plot_height < 220.0f) {
            plot_height = 220.0f;
        }

        if (ImPlot::BeginPlot("Spline interpolation", ImVec2(-1.0f, plot_height))) {
            ImPlot::SetupAxes("x", "y");
            if (spline_ready && data.spline_x.get_size() > 0) {
                ImPlot::PlotLine("Spline", &data.spline_x.get(0), &data.spline_y.get(0),
                                 data.spline_x.get_size());
            }
            if (data.node_x.get_size() > 0) {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                ImPlot::PlotScatter("Nodes", &data.node_x.get(0), &data.node_y.get(0),
                                    data.node_x.get_size());
            }
            ImPlot::EndPlot();
        }

        ImGui::Spacing();
        ImGui::BeginChild("PiecewiseSystem", ImVec2(0.0f, 0.0f), true);
        draw_piecewise_system(segments, spline_ready);
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
    delete segments;
    std::cout << "Graphical window closed. Returning to menu." << std::endl;
}
