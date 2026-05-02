#include "cubic_spline.h"
#include <cmath>
#include <vector>
#include <numbers>
#include <algorithm>
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
        std::vector<double> x_nodes;
        std::vector<double> f_nodes;
        std::vector<double> x_dense;
        std::vector<double> spline_vals;
    };

    // Инициализация без предупреждений
    DataSet data25;
    data25.step = 0.25;
    DataSet data50;
    data50.step = 0.5;

    // Заполнение узлов
    for (double x = 0.0; x <= PI + 1e-12; x += data25.step)
        data25.x_nodes.push_back(x);
    for (double x = 0.0; x <= PI + 1e-12; x += data50.step)
        data50.x_nodes.push_back(x);

    auto fill_f = [](DataSet& ds) {
        ds.f_nodes.resize(ds.x_nodes.size());
        for (size_t i = 0; i < ds.x_nodes.size(); ++i)
            ds.f_nodes[i] = std::sin(4.0 * ds.x_nodes[i]);
    };
    fill_f(data25);
    fill_f(data50);

    cubic_spline<double> spline25, spline50;
    spline25.build(data25.x_nodes.data(), data25.f_nodes.data(),
                   static_cast<int>(data25.x_nodes.size()));
    spline50.build(data50.x_nodes.data(), data50.f_nodes.data(),
                   static_cast<int>(data50.x_nodes.size()));

    const int N_DENSE = 400;
    auto fill_dense = [&](DataSet& ds, cubic_spline<double>& spline) {
        ds.x_dense.resize(N_DENSE);
        ds.spline_vals.resize(N_DENSE);
        for (int i = 0; i < N_DENSE; ++i) {
            double x = PI * i / (N_DENSE - 1);
            ds.x_dense[i] = x;
            ds.spline_vals[i] = spline.evaluate(x);
        }
    };
    fill_dense(data25, spline25);
    fill_dense(data50, spline50);

    std::vector<double> exact_dense(N_DENSE);
    for (int i = 0; i < N_DENSE; ++i)
        exact_dense[i] = std::sin(4.0 * data25.x_dense[i]);

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

        if (ImPlot::BeginPlot("##Exact")) {
            ImPlot::SetupAxes("x", "sin(4x)");
            ImPlot::PlotLine("sin(4x)", data25.x_dense.data(), exact_dense.data(), N_DENSE);
            ImPlot::EndPlot();
        }
        ImGui::SameLine();

        if (ImPlot::BeginPlot("##Step025")) {
            ImPlot::SetupAxes("x", "S(x) / sin(4x)");
            ImPlot::PlotLine("Exact", data25.x_dense.data(), exact_dense.data(), N_DENSE);
            ImPlot::PlotLine("Spline (step 0.25)", data25.x_dense.data(), data25.spline_vals.data(), N_DENSE);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            ImPlot::PlotScatter("Nodes", data25.x_nodes.data(), data25.f_nodes.data(),
                                static_cast<int>(data25.x_nodes.size()));
            ImPlot::EndPlot();
        }
        ImGui::SameLine();

        if (ImPlot::BeginPlot("##Step050")) {
            ImPlot::SetupAxes("x", "S(x) / sin(4x)");
            ImPlot::PlotLine("Exact", data25.x_dense.data(), exact_dense.data(), N_DENSE);
            ImPlot::PlotLine("Spline (step 0.5)", data50.x_dense.data(), data50.spline_vals.data(), N_DENSE);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            ImPlot::PlotScatter("Nodes", data50.x_nodes.data(), data50.f_nodes.data(),
                                static_cast<int>(data50.x_nodes.size()));
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