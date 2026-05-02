#include "cubic_spline.h"
#include "dynamic_array.h"
#include <cmath>
#include <numbers>
#include <iostream>

// Убираем GLFW_INCLUDE_NONE, чтобы GLFW сам включил нужный OpenGL заголовок
#include <GLFW/glfw3.h>

// Но на macOS нужен дополнительный флаг для подавления предупреждений об устаревании OpenGL
#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

void menu_spline_viewer() {
    std::cout << "\n=== Spline Viewer ===" << std::endl;
    std::cout << "Launching graphical window..." << std::endl;

    constexpr double PI = std::numbers::pi;

    struct DataSet {
        double step;
        DynamicArray<double> x_nodes;
        DynamicArray<double> f_nodes;
        DynamicArray<double> x_dense;
        DynamicArray<double> spline_vals;
        DynamicArray<double> exact_dense;
    };

    DataSet data;
    data.step = 0.5;

    int node_count = 0;
    for (double x = 0.0; x <= PI + 1e-12; x += data.step) {
        ++node_count;
    }

    data.x_nodes.resize(node_count);
    data.f_nodes.resize(node_count);
    int node_index = 0;
    for (double x = 0.0; x <= PI + 1e-12; x += data.step) {
        data.x_nodes.set(node_index, x);
        data.f_nodes.set(node_index, std::sin(4.0 * x));
        ++node_index;
    }

    cubic_spline<double> spline;
    spline.build(&data.x_nodes.get(0), &data.f_nodes.get(0), node_count);

    const int N_DENSE = 400;
    data.x_dense.resize(N_DENSE);
    data.spline_vals.resize(N_DENSE);
    data.exact_dense.resize(N_DENSE);
    for (int i = 0; i < N_DENSE; ++i) {
        double x = PI * i / (N_DENSE - 1);
        data.x_dense.set(i, x);
        data.spline_vals.set(i, spline.evaluate(x));
        data.exact_dense.set(i, std::sin(4.0 * x));
    }

    // Инициализация GLFW + ImGui + ImPlot
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW" << std::endl;
        return;
    }

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // На macOS обязательно
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Cubic Spline Interpolation", nullptr, nullptr);
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

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);
        ImGui::Begin("Spline vs sin(4x)", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        if (ImPlot::BeginPlot("##ExactAndSpline")) {
            ImPlot::SetupAxes("x", "y");
            ImPlot::PlotLine("sin(4x)", &data.x_dense[0], &data.exact_dense[0], N_DENSE);
            ImPlot::PlotLine("Spline (step 0.5)", &data.x_dense[0], &data.spline_vals[0], N_DENSE);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            ImPlot::PlotScatter("Nodes", &data.x_nodes[0], &data.f_nodes[0], data.x_nodes.get_size());
            ImPlot::EndPlot();
        }
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