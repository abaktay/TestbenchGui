#include <iostream>
#include <thread>
#include <chrono>

#include "uartComms.hpp"
#include "display.hpp"
#include "throttlePacket.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QTimer>
#include <QSplitter>
#include <QFrame>

class TestbenchWindow : public QMainWindow
{
    Q_OBJECT

public:
    TestbenchWindow(QWidget *parent = nullptr) : QMainWindow(parent), uart("/dev/ttyACM0")
    {
        setWindowTitle("Testbench GUI");
        setMinimumSize(1280, 720);
        
        setupUI();
        connectSignals();
        
        // Initialize UART
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        uart.start();
        
        // Renders the GUI at 60 FPS
        updateTimer = new QTimer(this);
        connect(updateTimer, &QTimer::timeout, this, &TestbenchWindow::updateTelemetry);
        updateTimer->start(16); 
    }
    
    ~TestbenchWindow()
    {
        uart.stop();
    }

private slots:
    void updateTelemetry()
    {
        // Update connection status
        if (uart.is_connected()) {
            statusLabel->setText("STATUS: CONNECTED");
            statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        } else {
            statusLabel->setText("STATUS: DISCONNECTED");
            statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
        }
        
        // Get telemetry data
        // uart.get_log(logs);
        

        float logs[5] = {10, 20, 30, 40, 50}; // -- TEST --
        
        // Updates telemetry display
        adc1Ch1Label->setText(QString("<div style='text-align: center;'>"
                                 "<span style='font-size: 10pt; color: #666;'>CH1</span><br>"
                                 "<span style='font-size: 18pt; font-weight: bold;'>%1</span>"
                                 "</div>").arg(logs[0], 0, 'f', 2));
        adc1Ch2Label->setText(QString("<div style='text-align: center;'>"
                                 "<span style='font-size: 10pt; color: #666;'>CH2</span><br>"
                                 "<span style='font-size: 18pt; font-weight: bold;'>%1</span>"
                                 "</div>").arg(logs[1], 0, 'f', 2));
        adc1Ch3Label->setText(QString("<div style='text-align: center;'>"
                                 "<span style='font-size: 10pt; color: #666;'>CH3</span><br>"
                                 "<span style='font-size: 18pt; font-weight: bold;'>%1</span>"
                                 "</div>").arg(logs[2], 0, 'f', 2));
        adc2Label->setText(QString("<div style='text-align: center;'>"
                                 "<span style='font-size: 18pt; font-weight: bold;'>%1</span>"
                                 "</div>").arg(logs[3], 0, 'f', 2));
        adc3Label->setText(QString("<div style='text-align: center;'>"
                                 "<span style='font-size: 18pt; font-weight: bold;'>%1</span>"
                                 "</div>").arg(logs[4], 0, 'f', 2));

                
        // std::cout << logs[0] << "\n";
        // std::cout << logs[1] << "\n";
        // std::cout << logs[2] << "\n";
        // std::cout << logs[3] << "\n";
        // std::cout << logs[4] << "\n";
    }
    
    void onThrottleSliderChanged(int value)
    {
        throttleSpinBox->setValue(value);
        throttleValueLabel->setText(QString("Throttle Value: %1").arg(value));
    }
    
    void onThrottleSpinBoxChanged(int value)
    {
        throttleSlider->setValue(value);
        throttleValueLabel->setText(QString("Throttle Value: %1").arg(value));
    }
    
    void sendPacket()
    {
        int throttleValue = throttleSlider->value();
        
        // Clamp value (should be unnecessary with Qt controls, but keeping for safety)
        if (throttleValue > 100) throttleValue = 100;
        if (throttleValue < 0) throttleValue = 0;
        
        // std::printf("throttle set to: %u\n", throttleValue);
        uart.set_throttle(static_cast<uint16_t>(throttleValue * 64 / 100));
    }
    
    void resetThrottle()
    {
        throttleSlider->setValue(0);
        throttleSpinBox->setValue(0);
        uart.set_throttle(0);
    }
    
    void armSystem()
    {
        uart.arm();
    }
    
    void disarmSystem()
    {
        uart.disarm();
    }
    
    void resetMicrocontroller()
    {
        uart.reset();
    }

private:
    void setupUI()
    {
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        
        // Status section
        createStatusSection(mainLayout);
        
        QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
        mainLayout->addWidget(splitter);
        
        // Telemetry section
        createTelemetrySection(splitter);
        
        // Control panel section
        createControlSection(splitter);
        
        // Set splitter proportions
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 1);
    }
    

    void createStatusSection(QVBoxLayout *parentLayout)
    {
        QFrame *statusFrame = new QFrame(this);
        statusFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
        statusFrame->setLineWidth(1);
        
        QHBoxLayout *statusLayout = new QHBoxLayout(statusFrame);
        statusLayout->setContentsMargins(3, 1, 3, 1);
        
        statusLabel = new QLabel("STATUS: DISCONNECTED", this);
        statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; font-size: 8pt; }");
        
        statusLayout->addWidget(statusLabel);
        statusLayout->addStretch();
        
        statusFrame->setFixedHeight(20);  
        parentLayout->addWidget(statusFrame);
    }
    
    void createTelemetrySection(QSplitter *splitter)
    {
        QGroupBox *telemetryGroup = new QGroupBox("Live Telemetry", this);
        QVBoxLayout *telemetryLayout = new QVBoxLayout(telemetryGroup);
        
        QHBoxLayout *mainLayout = new QHBoxLayout();
        mainLayout->setSpacing(20);
        mainLayout->setContentsMargins(10, 10, 10, 10);

        QGroupBox *adc1Group = new QGroupBox("ADC1", this);
        adc1Group->setStyleSheet(
            "QGroupBox {"
            "   border: 2px solid #3f51b5;"
            "   border-radius: 8px;"
            "   margin-top: 10px;"
            "}"
            "QGroupBox::title {"
            "   subcontrol-origin: margin;"
            "   subcontrol-position: top center;"
            "   color: #3f51b5;"
            "   font-weight: bold;"
            "   font-size: 12pt;"
            "}"
        );
        
        QVBoxLayout *adc1Layout = new QVBoxLayout(adc1Group);
        adc1Layout->setSpacing(10);
        
        // Add ADC1 channels
        createADCChannel(adc1Layout, "CHANNEL 1");
        createADCChannel(adc1Layout, "CHANNEL 2");
        createADCChannel(adc1Layout, "CHANNEL 3");

        // Create right side layout for ADC2 and ADC3 with group boxes
        QVBoxLayout *rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(15);
        
        // Create ADC2 Group Box
        QGroupBox *adc2Group = new QGroupBox("ADC2", this);
        adc2Group->setStyleSheet(
            "QGroupBox {"
            "   border: 2px solid #3f51b5;"
            "   border-radius: 8px;"
            "   margin-top: 10px;"
            "}"
            "QGroupBox::title {"
            "   subcontrol-origin: margin;"
            "   subcontrol-position: top center;"
            "   color: #3f51b5;"
            "   font-weight: bold;"
            "   font-size: 12pt;"
            "}"
        );
        
        QVBoxLayout *adc2Layout = new QVBoxLayout(adc2Group);
        adc2Layout->setSpacing(10);
        createADCChannel(adc2Layout, "ADC2");
        
        // Create ADC3 Group Box
        QGroupBox *adc3Group = new QGroupBox("ADC3", this);
        adc3Group->setStyleSheet(
            "QGroupBox {"
            "   border: 2px solid #3f51b5;"
            "   border-radius: 8px;"
            "   margin-top: 10px;"
            "}"
            "QGroupBox::title {"
            "   subcontrol-origin: margin;"
            "   subcontrol-position: top center;"
            "   color: #3f51b5;"
            "   font-weight: bold;"
            "   font-size: 12pt;"
            "}"
        );
        
        QVBoxLayout *adc3Layout = new QVBoxLayout(adc3Group);
        adc3Layout->setSpacing(10);
        createADCChannel(adc3Layout, "ADC3");

        // Add to right layout
        rightLayout->addWidget(adc2Group);
        rightLayout->addWidget(adc3Group);

        // Add to main layout
        mainLayout->addWidget(adc1Group, 2);  // 2/3 of space
        mainLayout->addLayout(rightLayout, 1); // 1/3 of space

        telemetryLayout->addLayout(mainLayout);
        telemetryLayout->addStretch();
        
        splitter->addWidget(telemetryGroup);
    }

    void createADCChannel(QLayout *parentLayout, const QString &title)
    {
        QFrame *channelFrame = new QFrame();
        channelFrame->setFrameShape(QFrame::StyledPanel);
        channelFrame->setStyleSheet(
            QString(
            "QFrame {"
            "   background-color: %1;"
            "   border-radius: 6px;"
            "   border: 1px solid %2;"
            "   padding: 8px;"
            "}"
            ).arg("#e3dcd1")  // Different bg colors
             .arg("#7986cb")  // Different border colors
        );
        
        QVBoxLayout *channelLayout = new QVBoxLayout(channelFrame);
        channelLayout->setContentsMargins(8, 8, 8, 8);

        // Don't show title labels for any channels since group box titles handle this
        // Value display
        QLabel *valueLabel = new QLabel("0.00");
        valueLabel->setStyleSheet(
            "QLabel {"
            "   font-family: 'Courier New';"
            "   font-size: 16pt;"
            "   color: #3e0b26;"
            "   qproperty-alignment: AlignCenter;"
            "}"
        );
        valueLabel->setMinimumWidth(80);

        channelLayout->addWidget(valueLabel);

        // Store reference to the value label
        if (title == "CHANNEL 1") adc1Ch1Label = valueLabel;
        else if (title == "CHANNEL 2") adc1Ch2Label = valueLabel;
        else if (title == "CHANNEL 3") adc1Ch3Label = valueLabel;
        else if (title == "ADC2") adc2Label = valueLabel;
        else if (title == "ADC3") adc3Label = valueLabel;

        parentLayout->addWidget(channelFrame);
    }
    
    void createControlSection(QSplitter *splitter)
    {
        QGroupBox *controlGroup = new QGroupBox("Control Panel", this);
        QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
        
        // Throttle control section
        throttleValueLabel = new QLabel("Throttle Value: 0", this);
        controlLayout->addWidget(throttleValueLabel);
        
        // Spin box for direct input
        throttleSpinBox = new QSpinBox(this);
        throttleSpinBox->setRange(0, 100);
        throttleSpinBox->setValue(0);
        controlLayout->addWidget(throttleSpinBox);
        
        // Add separator line
        QFrame *line1 = new QFrame(this);
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);
        controlLayout->addWidget(line1);
        
        // Slider for throttle
        throttleSlider = new QSlider(Qt::Horizontal, this);
        throttleSlider->setRange(0, 100);
        throttleSlider->setValue(0);
        throttleSlider->setTickPosition(QSlider::TicksBelow);
        throttleSlider->setTickInterval(10);
        controlLayout->addWidget(throttleSlider);
        
        // Control buttons
        QPushButton *sendPacketBtn = new QPushButton("Send Packet", this);
        QPushButton *resetThrottleBtn = new QPushButton("Reset Throttle", this);
        QPushButton *armBtn = new QPushButton("Arm", this);
        QPushButton *disarmBtn = new QPushButton("Disarm", this);
        QPushButton *resetMcuBtn = new QPushButton("Reset Microcontroller", this);
        
        // Style the arm/disarm buttons
        armBtn->setStyleSheet("QPushButton { background-color: #4CAF50; }");
        disarmBtn->setStyleSheet("QPushButton { background-color: #f44336; }");
        resetMcuBtn->setStyleSheet("QPushButton { background-color: #ff9800; }");
        
        controlLayout->addWidget(sendPacketBtn);
        controlLayout->addWidget(resetThrottleBtn);
        controlLayout->addWidget(armBtn);
        controlLayout->addWidget(disarmBtn);
        controlLayout->addWidget(resetMcuBtn);
        
        controlLayout->addStretch();
        
        // Store button references for signal connections
        this->sendPacketBtn = sendPacketBtn;
        this->resetThrottleBtn = resetThrottleBtn;
        this->armBtn = armBtn;
        this->disarmBtn = disarmBtn;
        this->resetMcuBtn = resetMcuBtn;
        
        splitter->addWidget(controlGroup);
    }
    
    void connectSignals()
    {
        connect(throttleSlider, &QSlider::valueChanged, this, &TestbenchWindow::onThrottleSliderChanged);
        connect(throttleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TestbenchWindow::onThrottleSpinBoxChanged);
        
        connect(sendPacketBtn, &QPushButton::clicked, this, &TestbenchWindow::sendPacket);
        connect(resetThrottleBtn, &QPushButton::clicked, this, &TestbenchWindow::resetThrottle);
        connect(armBtn, &QPushButton::clicked, this, &TestbenchWindow::armSystem);
        connect(disarmBtn, &QPushButton::clicked, this, &TestbenchWindow::disarmSystem);
        connect(resetMcuBtn, &QPushButton::clicked, this, &TestbenchWindow::resetMicrocontroller);
    }
    
    // UI Components
    QLabel *statusLabel;
    QLabel *throttleValueLabel;
    QLabel *adc1Ch1Label;
    QLabel *adc1Ch2Label;
    QLabel *adc1Ch3Label;
    QLabel *adc2Label;
    QLabel *adc3Label;
    QSlider *throttleSlider;
    QSpinBox *throttleSpinBox;
    QPushButton *sendPacketBtn;
    QPushButton *resetThrottleBtn;
    QPushButton *armBtn;
    QPushButton *disarmBtn;
    QPushButton *resetMcuBtn;
    
    // Timer and UART
    QTimer *updateTimer;
    UARTComms uart;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TestbenchWindow window;
    window.show();
    
    return app.exec();
}

#include "main.moc"
