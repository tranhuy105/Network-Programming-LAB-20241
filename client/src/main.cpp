#include "../lib/imgui/imgui.h"
#include "MainUI.h"
#include "../lib/imgui/backends/imgui_impl_glfw.h"
#include "../lib/imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <memory>
#include "../include/DeviceProxy.h"
#include "../include/HomeManager.h"

int main() {
    // 1. Khởi tạo GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // 2. Tạo cửa sổ và ngữ cảnh OpenGL
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Device Scanner UI", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync

    // 3. Khởi tạo Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5f);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 4. Biến dữ liệu
    HomeManager homeManager; // Quản lý các Home và Room
    std::vector<std::shared_ptr<DeviceProxy>> devices; // Danh sách thiết bị quét được
    std::string clientId = "JACK97"; // Client ID

    // Tải dữ liệu từ file
    homeManager.loadFromFile();

    // 5. Vòng lặp chính
    while (!glfwWindowShouldClose(window)) {
        // Bắt đầu frame mới của Dear ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Gọi hàm để vẽ giao diện
        renderMainUI(homeManager, devices, clientId);

        // Kết thúc frame
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffer và xử lý sự kiện
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Lưu dữ liệu trước khi thoát
    homeManager.saveToFile();

    // 6. Cleanup Dear ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 7. Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
