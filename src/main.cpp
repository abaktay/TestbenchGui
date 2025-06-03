#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <imgui.h>
#include <iostream>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "uartComms.hpp"
#include "display.hpp"
#include "throttlePacket.hpp"

int main() {
    if (!glfwInit()) return 1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Testbench GUI", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();

    UARTComms uart("/dev/ttyUSB1");

    uart.start();

    int slider_value = 0;
    
    float logs[5] = {};

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        ImGui::Begin("Status: ", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::SameLine();
        if (uart.is_connected()) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));  // Green
            ImGui::Text("CONNECTED");
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));  // Red
            ImGui::Text("DISCONNECTED");
        }
        ImGui::PopStyleColor();
        ImGui::End();

        ImGui::Begin("Live Telemetry", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::SameLine();
        
        ImGui::Separator();
        //ImGui::Text("Received Data (%.1f FPS)", io.Framerate);
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
        

        uart.get_log(logs);

        ImGui::Text("ADC1 CHANNEL 1\t%.2f", logs[0]);
        ImGui::Text("ADC1 CHANNEL 2\t%.2f", logs[1]);
        ImGui::Text("ADC1 CHANNEL 3\t%.2f", logs[2]);
        ImGui::Text("ADC2\t%0.2f", logs[3]);
        ImGui::Text("ADC3\t%0.2f", logs[4]);
        
      /*  std::cout << logs[0] << "\n";
        std::cout << logs[1] << "\n";
        std::cout << logs[2] << "\n";
        std::cout << logs[3] << "\n";
        std::cout << logs[4] << "\n";*/
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        
        ImGui::EndChild();
        ImGui::End();


        ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoCollapse);
        {
            ImGui::Text("Throttle Value: %d", slider_value);
            ImGui::InputInt("##xx", &slider_value, 0, 100, 0);
            ImGui::Separator();
            ImGui::SliderInt("##xxxx", &slider_value, 0, 100, "%d");

            if (ImGui::Button("Send Packet")) {
               // arm_status = 0b10 
                if (slider_value > 100) {
                    slider_value = 100;
                }
                if (slider_value < 0) {
                    slider_value = 0;
                }
                uart.set_throttle(slider_value*64/100);

            }

            if (ImGui::Button("Reset throttle")) {
               // arm_status = 0b10 
                slider_value = 0;  
                uart.set_throttle(slider_value*64/100);
            }

            if (ImGui::Button("Arm")) {
               // arm_status = 0b01 
                uart.arm();
            }

            if (ImGui::Button("Disarm")) {
               // arm_status = 0b00 
                uart.disarm(); 
            }

            if (ImGui::Button("Reset microcontroller")) {
               // arm_status = 0b11 
               uart.reset();
            }
        }
        ImGui::End();

        ImGui::Render();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // 60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    uart.stop();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


