#include "cubic_spline.h"
#include "dynamic_array.h"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
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

constexpr int kMinSplinePoints = 3;
constexpr int kPlotSamples = 500;

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

void collect_nodes(const CubicSpline<double> &spline, PlotData &data) {
    const int count = spline.get_count();
    data.node_x.resize(count);
    data.node_y.resize(count);

    for (int index = 0; index < count; ++index) {
        const Point<double> &point = spline.get(index);
        data.node_x.set(index, point.x);
        data.node_y.set(index, point.y);
    }
}

bool is_valid_append(const CubicSpline<double> &spline, const Point<double> &point,
                     std::string &message) {
    if (spline.get_count() > 0 && point.x <= spline.get_last().x) {
        message = "X for end insertion must be greater than the last point X";
        return false;
    }
    return true;
}

bool is_valid_prepend(const CubicSpline<double> &spline, const Point<double> &point,
                      std::string &message) {
    if (spline.get_count() > 0 && point.x >= spline.get_first().x) {
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

bool rebuild_spline(CubicSpline<double> &spline, std::string &message) {
    const int count = spline.get_count();
    if (count < kMinSplinePoints) {
        message = "Add at least three points before building the spline";
        return false;
    }

    DynamicArray<Point<double>> points(count);
    for (int index = 0; index < count; ++index) {
        points.set(index, spline.get(index));
        if (index > 0 && points.get(index).x <= points.get(index - 1).x) {
            message = "Point X values must be strictly increasing";
            return false;
        }
    }

    try {
        spline.build(&points.get(0), count);
    } catch (const std::exception &error) {
        message = error.what();
        return false;
    }

    message = "Spline rebuilt";
    return true;
}

bool refresh_plot(CubicSpline<double> &spline, PlotData &data, std::string &message) {
    collect_nodes(spline, data);
    data.spline_x.resize(0);
    data.spline_y.resize(0);

    if (spline.get_count() < kMinSplinePoints) {
        return false;
    }

    const Point<double> &first = spline.get_first();
    const Point<double> &last = spline.get_last();
    if (first.x >= last.x) {
        message = "Cannot plot a spline with an invalid X range";
        return false;
    }

    data.spline_x.resize(kPlotSamples);
    data.spline_y.resize(kPlotSamples);
    for (int index = 0; index < kPlotSamples; ++index) {
        const double t = static_cast<double>(index) / static_cast<double>(kPlotSamples - 1);
        const double x = first.x + (last.x - first.x) * t;
        data.spline_x.set(index, x);
        data.spline_y.set(index, spline.evaluate(x));
    }

    return true;
}

bool add_point(CubicSpline<double> &spline, const Point<double> &point, bool to_beginning,
               std::string &message) {
    if (to_beginning) {
        if (!is_valid_prepend(spline, point, message)) {
            return false;
        }
    } else if (!is_valid_append(spline, point, message)) {
        return false;
    }

    try {
        if (to_beginning) {
            spline.prepend(point);
            message = "Point added to the beginning";
        } else {
            spline.append(point);
            message = "Point added to the end";
        }
    } catch (const std::exception &error) {
        message = error.what();
        return false;
    }

    if (spline.get_count() >= kMinSplinePoints) {
        return rebuild_spline(spline, message);
    }
    return true;
}

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

} // namespace

void menu_spline_viewer() {
    std::cout << "\n=== Spline Viewer ===" << std::endl;
    std::cout << "Launching graphical window..." << std::endl;

    std::unique_ptr<CubicSpline<double>> spline = std::make_unique<CubicSpline<double>>();
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

    // Главный цикл окна
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImPlot::NewFrame() больше не нужна – оставляем только ImGui::NewFrame()

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        ImGui::Begin("Cubic Spline Interpolation", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        ImGui::BeginChild("Controls", ImVec2(330.0f, 0.0f), true);
        ImGui::Text("CubicSpline points");
        ImGui::Separator();

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("x", input_x, sizeof(input_x));
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("y", input_y, sizeof(input_y));

        if (ImGui::Button("Add begin", ImVec2(-1.0f, 0.0f))) {
            Point<double> point{};
            if (parse_point_input(input_x, input_y, point, status)) {
                add_point(*spline, point, true, status);
                spline_ready = refresh_plot(*spline, data, status);
            }
        }
        if (ImGui::Button("Add end", ImVec2(-1.0f, 0.0f))) {
            Point<double> point{};
            if (parse_point_input(input_x, input_y, point, status)) {
                add_point(*spline, point, false, status);
                spline_ready = refresh_plot(*spline, data, status);
            }
        }
        if (ImGui::Button("Rebuild spline", ImVec2(-1.0f, 0.0f))) {
            spline_ready = rebuild_spline(*spline, status);
            if (spline_ready) {
                spline_ready = refresh_plot(*spline, data, status);
            }
        }
        if (ImGui::Button("Clear", ImVec2(-1.0f, 0.0f))) {
            spline = std::make_unique<CubicSpline<double>>();
            clear_plot_data(data);
            spline_ready = false;
            status = "Create a point sequence";
        }

        ImGui::Separator();
        ImGui::TextWrapped("%s", status.c_str());
        ImGui::Text("Points: %d", spline->get_count());
        draw_points_table(data);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Plot", ImVec2(0.0f, 0.0f), false);
        if (ImPlot::BeginPlot("Spline interpolation", ImVec2(-1.0f, -1.0f))) {
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
    std::cout << "Graphical window closed. Returning to menu." << std::endl;
}
