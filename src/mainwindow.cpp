#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <QColorDialog>
#include <QScrollArea>
#include <iostream>

void MainWindow::initialize() {
    // Create tab widget
    tabWidget = new QTabWidget(this);
    this->setLayout(new QHBoxLayout());
    this->layout()->addWidget(tabWidget);
    
    // Create render tab (existing functionality)
    renderTab = new QWidget();
    realtime = new Realtime;
    aspectRatioWidget = new AspectRatioWidget(renderTab);
    aspectRatioWidget->setAspectWidget(realtime, 3.f/4.f);
    QHBoxLayout *hLayout = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setAlignment(Qt::AlignTop);
    
    QWidget *scrollWidget = new QWidget();
    scrollWidget->setLayout(vLayout);
    
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMinimumWidth(280);
    scrollArea->setMaximumWidth(300);
    scrollArea->setMaximumHeight(500);
    
    hLayout->addWidget(scrollArea);
    hLayout->addWidget(aspectRatioWidget, 1);
    renderTab->setLayout(hLayout);
    
    // Create map builder tab
    mapBuilderWidget = new MapBuilderWidget();
    
    // Connect map to realtime - set the map as active when it's created/updated
    // The map is automatically updated when maps are generated, so we set it initially
    realtime->setActiveMap(mapBuilderWidget->getMap());
    
    // Add tabs
    tabWidget->addTab(renderTab, "Render");
    tabWidget->addTab(mapBuilderWidget, "Map Builder");
    
    // Create labels in sidebox
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    QLabel *tesselation_label = new QLabel();
    tesselation_label->setText("Tesselation");
    tesselation_label->setFont(font);
    QLabel *camera_label = new QLabel();
    camera_label->setText("Camera");
    camera_label->setFont(font);

    // Create telemetry labels
    QLabel *telemetry_label = new QLabel();
    telemetry_label->setText("Telemetry");
    telemetry_label->setFont(font);
    
    cameraPosLabel = new QLabel();
    cameraPosLabel->setText("Position: (0.00, 0.00, 0.00)");
    cameraPosLabel->setWordWrap(true);
    
    chunkPosLabel = new QLabel();
    chunkPosLabel->setText("Chunk: (0, 0)");
    chunkPosLabel->setWordWrap(true);

    // Create camera controls
    QLabel *cameraControls_label = new QLabel();
    cameraControls_label->setText("Camera Controls");
    cameraControls_label->setFont(font);
    
    flyingModeCheckbox = new QCheckBox();
    flyingModeCheckbox->setText(QStringLiteral("Flying Mode"));
    flyingModeCheckbox->setChecked(true);
    
    QLabel *movementSpeed_label = new QLabel();
    movementSpeed_label->setText("Movement Speed:");
    
    movementSpeedSlider = new QSlider(Qt::Orientation::Horizontal);
    movementSpeedSlider->setTickInterval(1);
    movementSpeedSlider->setMinimum(100);
    movementSpeedSlider->setMaximum(1000);
    movementSpeedSlider->setValue(100);
    movementSpeedSlider->setMaximumWidth(160);
    
    movementSpeedBox = new QDoubleSpinBox();
    movementSpeedBox->setMinimum(1.0);
    movementSpeedBox->setMaximum(10.0);
    movementSpeedBox->setSingleStep(0.1);
    movementSpeedBox->setDecimals(2);
    movementSpeedBox->setValue(1.0);
    movementSpeedBox->setFixedWidth(70);
    
    QLabel *jumpHeight_label = new QLabel();
    jumpHeight_label->setText("Jump Height:");
    
    jumpHeightSlider = new QSlider(Qt::Orientation::Horizontal);
    jumpHeightSlider->setTickInterval(1);
    jumpHeightSlider->setMinimum(100);
    jumpHeightSlider->setMaximum(1000);
    jumpHeightSlider->setValue(100);
    jumpHeightSlider->setMaximumWidth(160);
    
    jumpHeightBox = new QDoubleSpinBox();
    jumpHeightBox->setMinimum(1.0);
    jumpHeightBox->setMaximum(10.0);
    jumpHeightBox->setSingleStep(0.1);
    jumpHeightBox->setDecimals(2);
    jumpHeightBox->setValue(1.0);
    jumpHeightBox->setFixedWidth(70);
    
    QLabel *cameraHeight_label = new QLabel();
    cameraHeight_label->setText("Camera Height:");
    
    cameraHeightSlider = new QSlider(Qt::Orientation::Horizontal);
    cameraHeightSlider->setTickInterval(1);
    cameraHeightSlider->setMinimum(25);
    cameraHeightSlider->setMaximum(300);
    cameraHeightSlider->setValue(100);
    cameraHeightSlider->setMaximumWidth(160);
    
    cameraHeightBox = new QDoubleSpinBox();
    cameraHeightBox->setMinimum(0.25);
    cameraHeightBox->setMaximum(3.0);
    cameraHeightBox->setSingleStep(0.05);
    cameraHeightBox->setDecimals(2);
    cameraHeightBox->setValue(1.0);
    cameraHeightBox->setFixedWidth(70);
    
    QLabel *gravity_label = new QLabel();
    gravity_label->setText("Gravity:");
    
    gravitySlider = new QSlider(Qt::Orientation::Horizontal);
    gravitySlider->setTickInterval(1);
    gravitySlider->setMinimum(0);
    gravitySlider->setMaximum(200);
    gravitySlider->setValue(100);
    gravitySlider->setMaximumWidth(160);
    
    gravityBox = new QDoubleSpinBox();
    gravityBox->setMinimum(0.0);
    gravityBox->setMaximum(2.0);
    gravityBox->setSingleStep(0.1);
    gravityBox->setDecimals(2);
    gravityBox->setValue(1.0);
    gravityBox->setFixedWidth(70);
    
    QLabel *overheadLight_label = new QLabel();
    overheadLight_label->setText("Overhead Light:");
    
    overheadLightSlider = new QSlider(Qt::Orientation::Horizontal);
    overheadLightSlider->setTickInterval(1);
    overheadLightSlider->setMinimum(0);
    overheadLightSlider->setMaximum(200);
    overheadLightSlider->setValue(100);
    overheadLightSlider->setMaximumWidth(160);
    
    overheadLightBox = new QDoubleSpinBox();
    overheadLightBox->setMinimum(0.0);
    overheadLightBox->setMaximum(2.0);
    overheadLightBox->setSingleStep(0.1);
    overheadLightBox->setDecimals(2);
    overheadLightBox->setValue(1.0);
    overheadLightBox->setFixedWidth(70);
    
    QLabel *playerColor_label = new QLabel();
    playerColor_label->setText("Player Light Color:");
    
    playerColorButton = new QPushButton();
    playerColorButton->setText("White");
    playerColorButton->setStyleSheet("background-color: white; border: 1px solid black;");
    playerColorButton->setMinimumHeight(30);

    QLabel *ec_label = new QLabel();
    ec_label->setText("Extra Credit");
    ec_label->setFont(font);
    QLabel *param1_label = new QLabel();
    param1_label->setText("Parameter 1:");
    QLabel *param2_label = new QLabel();
    param2_label->setText("Parameter 2:");
    QLabel *near_label = new QLabel();
    near_label->setText("Near Plane:");
    QLabel *far_label = new QLabel();
    far_label->setText("Far Plane:");

    // Create file uploader for scene file
    uploadFile = new QPushButton();
    uploadFile->setText(QStringLiteral("Upload Scene File"));
    
    saveImage = new QPushButton();
    saveImage->setText(QStringLiteral("Save Image"));

    // Creates the boxes containing the parameter sliders and number boxes
    QGroupBox *p1Layout = new QGroupBox();
    QHBoxLayout *l1 = new QHBoxLayout();
    QGroupBox *p2Layout = new QGroupBox();
    QHBoxLayout *l2 = new QHBoxLayout();

    // Create slider controls to control parameters
    p1Slider = new QSlider(Qt::Orientation::Horizontal);
    p1Slider->setTickInterval(1);
    p1Slider->setMinimum(1);
    p1Slider->setMaximum(25);
    p1Slider->setValue(1);
    p1Slider->setMaximumWidth(160);

    p1Box = new QSpinBox();
    p1Box->setMinimum(1);
    p1Box->setMaximum(25);
    p1Box->setSingleStep(1);
    p1Box->setValue(1);
    p1Box->setFixedWidth(70);

    p2Slider = new QSlider(Qt::Orientation::Horizontal);
    p2Slider->setTickInterval(1);
    p2Slider->setMinimum(1);
    p2Slider->setMaximum(25);
    p2Slider->setValue(1);
    p2Slider->setMaximumWidth(160);

    p2Box = new QSpinBox();
    p2Box->setMinimum(1);
    p2Box->setMaximum(25);
    p2Box->setSingleStep(1);
    p2Box->setValue(1);
    p2Box->setFixedWidth(70);

    // Adds the slider and number box to the parameter layouts
    l1->addWidget(p1Slider);
    l1->addWidget(p1Box);
    p1Layout->setLayout(l1);

    l2->addWidget(p2Slider);
    l2->addWidget(p2Box);
    p2Layout->setLayout(l2);

    // Creates the boxes containing the camera sliders and number boxes
    QGroupBox *nearLayout = new QGroupBox();
    QHBoxLayout *lnear = new QHBoxLayout();
    QGroupBox *farLayout = new QGroupBox();
    QHBoxLayout *lfar = new QHBoxLayout();

    // Create slider controls to control near/far planes
    nearSlider = new QSlider(Qt::Orientation::Horizontal);
    nearSlider->setTickInterval(1);
    nearSlider->setMinimum(1);
    nearSlider->setMaximum(1000);
    nearSlider->setValue(10);
    nearSlider->setMaximumWidth(160);

    nearBox = new QDoubleSpinBox();
    nearBox->setMinimum(0.01f);
    nearBox->setMaximum(10.f);
    nearBox->setSingleStep(0.1f);
    nearBox->setValue(0.1f);
    nearBox->setFixedWidth(70);

    farSlider = new QSlider(Qt::Orientation::Horizontal);
    farSlider->setTickInterval(1);
    farSlider->setMinimum(1000);
    farSlider->setMaximum(50000); // 500 * 100 to match spinbox max of 500
    farSlider->setValue(10000);
    farSlider->setMaximumWidth(160);

    farBox = new QDoubleSpinBox();
    farBox->setMinimum(10.f);
    farBox->setMaximum(500.f);
    farBox->setSingleStep(0.1f);
    farBox->setValue(100.f);
    farBox->setFixedWidth(70);

    // Adds the slider and number box to the parameter layouts
    lnear->addWidget(nearSlider);
    lnear->addWidget(nearBox);
    nearLayout->setLayout(lnear);

    lfar->addWidget(farSlider);
    lfar->addWidget(farBox);
    farLayout->setLayout(lfar);

    // Extra Credit:
    ec1 = new QCheckBox();
    ec1->setText(QStringLiteral("Extra Credit 1"));
    ec1->setChecked(false);

    ec2 = new QCheckBox();
    ec2->setText(QStringLiteral("Extra Credit 2"));
    ec2->setChecked(false);

    ec3 = new QCheckBox();
    ec3->setText(QStringLiteral("Extra Credit 3"));
    ec3->setChecked(false);

    ec4 = new QCheckBox();
    ec4->setText(QStringLiteral("Extra Credit 4"));
    ec4->setChecked(false);

    vLayout->addWidget(uploadFile);
    vLayout->addWidget(saveImage);
    vLayout->addWidget(tesselation_label);
    vLayout->addWidget(param1_label);
    vLayout->addWidget(p1Layout);
    vLayout->addWidget(param2_label);
    vLayout->addWidget(p2Layout);
    vLayout->addWidget(camera_label);
    vLayout->addWidget(near_label);
    vLayout->addWidget(nearLayout);
    vLayout->addWidget(far_label);
    vLayout->addWidget(farLayout);
    
    // Add telemetry section
    vLayout->addWidget(telemetry_label);
    vLayout->addWidget(cameraPosLabel);
    vLayout->addWidget(chunkPosLabel);
    
    // Add camera controls
    vLayout->addWidget(cameraControls_label);
    vLayout->addWidget(flyingModeCheckbox);
    
    QGroupBox *movementSpeedLayout = new QGroupBox();
    QHBoxLayout *movementLayout = new QHBoxLayout();
    movementLayout->addWidget(movementSpeed_label);
    movementLayout->addWidget(movementSpeedSlider);
    movementLayout->addWidget(movementSpeedBox);
    movementSpeedLayout->setLayout(movementLayout);
    vLayout->addWidget(movementSpeedLayout);
    
    QGroupBox *jumpHeightLayout = new QGroupBox();
    QHBoxLayout *jumpLayout = new QHBoxLayout();
    jumpLayout->addWidget(jumpHeight_label);
    jumpLayout->addWidget(jumpHeightSlider);
    jumpLayout->addWidget(jumpHeightBox);
    jumpHeightLayout->setLayout(jumpLayout);
    vLayout->addWidget(jumpHeightLayout);
    
    QGroupBox *cameraHeightLayout = new QGroupBox();
    QHBoxLayout *cameraLayout = new QHBoxLayout();
    cameraLayout->addWidget(cameraHeight_label);
    cameraLayout->addWidget(cameraHeightSlider);
    cameraLayout->addWidget(cameraHeightBox);
    cameraHeightLayout->setLayout(cameraLayout);
    vLayout->addWidget(cameraHeightLayout);
    
    QGroupBox *gravityLayout = new QGroupBox();
    QHBoxLayout *gravityLayout_inner = new QHBoxLayout();
    gravityLayout_inner->addWidget(gravity_label);
    gravityLayout_inner->addWidget(gravitySlider);
    gravityLayout_inner->addWidget(gravityBox);
    gravityLayout->setLayout(gravityLayout_inner);
    vLayout->addWidget(gravityLayout);
    
    QGroupBox *overheadLightLayout = new QGroupBox();
    QHBoxLayout *overheadLightLayout_inner = new QHBoxLayout();
    overheadLightLayout_inner->addWidget(overheadLight_label);
    overheadLightLayout_inner->addWidget(overheadLightSlider);
    overheadLightLayout_inner->addWidget(overheadLightBox);
    overheadLightLayout->setLayout(overheadLightLayout_inner);
    vLayout->addWidget(overheadLightLayout);
    
    QGroupBox *playerColorLayout = new QGroupBox();
    QHBoxLayout *colorLayout = new QHBoxLayout();
    colorLayout->addWidget(playerColor_label);
    colorLayout->addWidget(playerColorButton);
    playerColorLayout->setLayout(colorLayout);
    vLayout->addWidget(playerColorLayout);

    // Extra Credit:
    vLayout->addWidget(ec_label);
    vLayout->addWidget(ec1);
    vLayout->addWidget(ec2);
    vLayout->addWidget(ec3);
    vLayout->addWidget(ec4);

    connectUIElements();

    // Set default values of 5 for tesselation parameters
    onValChangeP1(5);
    onValChangeP2(5);

    // Set default values for near and far planes
    onValChangeNearBox(0.1f);
    onValChangeFarBox(10.f);
}

void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

void MainWindow::connectUIElements() {
    // From old Project 6
    //connectPerPixelFilter();
    //connectKernelBasedFilter();
    connectUploadFile();
    connectSaveImage();
    connectParam1();
    connectParam2();
    connectNear();
    connectFar();
    connectExtraCredit();
    
    // Connect telemetry signal from realtime to update labels
    connect(realtime, &Realtime::telemetryUpdate, this, &MainWindow::onTelemetryUpdate);
    
    // Connect camera controls
    connect(flyingModeCheckbox, &QCheckBox::clicked, this, &MainWindow::onFlyingModeChanged);
    connect(movementSpeedSlider, &QSlider::valueChanged, this, &MainWindow::onMovementSpeedChanged);
    connect(movementSpeedBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onMovementSpeedBoxChanged);
    connect(jumpHeightSlider, &QSlider::valueChanged, this, &MainWindow::onJumpHeightChanged);
    connect(jumpHeightBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onJumpHeightBoxChanged);
    connect(cameraHeightSlider, &QSlider::valueChanged, this, &MainWindow::onCameraHeightChanged);
    connect(cameraHeightBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onCameraHeightBoxChanged);
    connect(gravitySlider, &QSlider::valueChanged, this, &MainWindow::onGravityChanged);
    connect(gravityBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onGravityBoxChanged);
    connect(overheadLightSlider, &QSlider::valueChanged, this, &MainWindow::onOverheadLightChanged);
    connect(overheadLightBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onOverheadLightBoxChanged);
    connect(playerColorButton, &QPushButton::clicked, this, &MainWindow::onPlayerColorChanged);
}


// From old Project 6
// void MainWindow::connectPerPixelFilter() {
//     connect(filter1, &QCheckBox::clicked, this, &MainWindow::onPerPixelFilter);
// }
// void MainWindow::connectKernelBasedFilter() {
//     connect(filter2, &QCheckBox::clicked, this, &MainWindow::onKernelBasedFilter);
// }

void MainWindow::connectUploadFile() {
    connect(uploadFile, &QPushButton::clicked, this, &MainWindow::onUploadFile);
}

void MainWindow::connectSaveImage() {
    connect(saveImage, &QPushButton::clicked, this, &MainWindow::onSaveImage);
}

void MainWindow::connectParam1() {
    connect(p1Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP1);
    connect(p1Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP1);
}

void MainWindow::connectParam2() {
    connect(p2Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP2);
    connect(p2Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP2);
}

void MainWindow::connectNear() {
    connect(nearSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeNearSlider);
    connect(nearBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeNearBox);
}

void MainWindow::connectFar() {
    connect(farSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeFarSlider);
    connect(farBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeFarBox);
}

void MainWindow::connectExtraCredit() {
    connect(ec1, &QCheckBox::clicked, this, &MainWindow::onExtraCredit1);
    connect(ec2, &QCheckBox::clicked, this, &MainWindow::onExtraCredit2);
    connect(ec3, &QCheckBox::clicked, this, &MainWindow::onExtraCredit3);
    connect(ec4, &QCheckBox::clicked, this, &MainWindow::onExtraCredit4);
}

// From old Project 6
// void MainWindow::onPerPixelFilter() {
//     settings.perPixelFilter = !settings.perPixelFilter;
//     realtime->settingsChanged();
// }
// void MainWindow::onKernelBasedFilter() {
//     settings.kernelBasedFilter = !settings.kernelBasedFilter;
//     realtime->settingsChanged();
// }

void MainWindow::onUploadFile() {
    // Get abs path of scene file
    QString configFilePath = QFileDialog::getOpenFileName(this, tr("Upload File"),
                                                          QDir::currentPath()
                                                              .append(QDir::separator())
                                                              .append("scenefiles")
                                                              .append(QDir::separator())
                                                              .append("realtime")
                                                              .append(QDir::separator())
                                                              .append("required"), tr("Scene Files (*.json)"));
    if (configFilePath.isNull()) {
        std::cout << "Failed to load null scenefile." << std::endl;
        return;
    }

    settings.sceneFilePath = configFilePath.toStdString();

    std::cout << "Loaded scenefile: \"" << configFilePath.toStdString() << "\"." << std::endl;

    realtime->sceneChanged();
}

void MainWindow::onSaveImage() {
    if (settings.sceneFilePath.empty()) {
        std::cout << "No scene file loaded." << std::endl;
        return;
    }
    std::string sceneName = settings.sceneFilePath.substr(0, settings.sceneFilePath.find_last_of("."));
    sceneName = sceneName.substr(sceneName.find_last_of("/")+1);
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Image"),
                                                    QDir::currentPath()
                                                        .append(QDir::separator())
                                                        .append("student_outputs")
                                                        .append(QDir::separator())
                                                        .append("realtime")
                                                        .append(QDir::separator())
                                                        .append("required")
                                                        .append(QDir::separator())
                                                        .append(sceneName), tr("Image Files (*.png)"));
    std::cout << "Saving image to: \"" << filePath.toStdString() << "\"." << std::endl;
    realtime->saveViewportImage(filePath.toStdString());
}

void MainWindow::onValChangeP1(int newValue) {
    p1Slider->setValue(newValue);
    p1Box->setValue(newValue);
    settings.shapeParameter1 = p1Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeP2(int newValue) {
    p2Slider->setValue(newValue);
    p2Box->setValue(newValue);
    settings.shapeParameter2 = p2Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearSlider(int newValue) {
    //nearSlider->setValue(newValue);
    nearBox->setValue(newValue/100.f);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarSlider(int newValue) {
    //farSlider->setValue(newValue);
    farBox->setValue(newValue/100.f);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearBox(double newValue) {
    nearSlider->setValue(int(newValue*100.f));
    //nearBox->setValue(newValue);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarBox(double newValue) {
    farSlider->setValue(int(newValue*100.f));
    //farBox->setValue(newValue);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

// Extra Credit:

void MainWindow::onExtraCredit1() {
    settings.extraCredit1 = !settings.extraCredit1;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit2() {
    settings.extraCredit2 = !settings.extraCredit2;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit3() {
    settings.extraCredit3 = !settings.extraCredit3;
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit4() {
    settings.extraCredit4 = !settings.extraCredit4;
    realtime->settingsChanged();
}

void MainWindow::onTelemetryUpdate(float x, float y, float z, int chunkX, int chunkZ) {
    QString posText = QString("Position: (%1, %2, %3)")
                      .arg(x, 0, 'f', 2)
                      .arg(y, 0, 'f', 2)
                      .arg(z, 0, 'f', 2);
    cameraPosLabel->setText(posText);
    
    QString chunkText = QString("Chunk: (%1, %2)")
                        .arg(chunkX)
                        .arg(chunkZ);
    chunkPosLabel->setText(chunkText);
}

void MainWindow::onFlyingModeChanged() {
    bool flying = flyingModeCheckbox->isChecked();
    realtime->setFlyingMode(flying);
}

void MainWindow::onMovementSpeedChanged(int value) {
    double multiplier = value / 100.0;
    movementSpeedBox->blockSignals(true);
    movementSpeedBox->setValue(multiplier);
    movementSpeedBox->blockSignals(false);
    realtime->setMovementSpeedMultiplier(multiplier);
}

void MainWindow::onMovementSpeedBoxChanged(double value) {
    int sliderValue = static_cast<int>(value * 100);
    movementSpeedSlider->blockSignals(true);
    movementSpeedSlider->setValue(sliderValue);
    movementSpeedSlider->blockSignals(false);
    realtime->setMovementSpeedMultiplier(value);
}

void MainWindow::onJumpHeightChanged(int value) {
    double multiplier = value / 100.0;
    jumpHeightBox->blockSignals(true);
    jumpHeightBox->setValue(multiplier);
    jumpHeightBox->blockSignals(false);
    realtime->setJumpHeightMultiplier(multiplier);
}

void MainWindow::onJumpHeightBoxChanged(double value) {
    int sliderValue = static_cast<int>(value * 100);
    jumpHeightSlider->blockSignals(true);
    jumpHeightSlider->setValue(sliderValue);
    jumpHeightSlider->blockSignals(false);
    realtime->setJumpHeightMultiplier(value);
}

void MainWindow::onCameraHeightChanged(int value) {
    double multiplier = value / 100.0;
    cameraHeightBox->blockSignals(true);
    cameraHeightBox->setValue(multiplier);
    cameraHeightBox->blockSignals(false);
    realtime->setCameraHeightMultiplier(multiplier);
}

void MainWindow::onCameraHeightBoxChanged(double value) {
    int sliderValue = static_cast<int>(value * 100);
    cameraHeightSlider->blockSignals(true);
    cameraHeightSlider->setValue(sliderValue);
    cameraHeightSlider->blockSignals(false);
    realtime->setCameraHeightMultiplier(value);
}

void MainWindow::onGravityChanged(int value) {
    double multiplier = value / 100.0;
    gravityBox->blockSignals(true);
    gravityBox->setValue(multiplier);
    gravityBox->blockSignals(false);
    realtime->setGravityMultiplier(multiplier);
}

void MainWindow::onGravityBoxChanged(double value) {
    int sliderValue = static_cast<int>(value * 100.0);
    gravitySlider->blockSignals(true);
    gravitySlider->setValue(sliderValue);
    gravitySlider->blockSignals(false);
    realtime->setGravityMultiplier(value);
}

void MainWindow::onOverheadLightChanged(int value) {
    double intensity = value / 100.0;
    overheadLightBox->blockSignals(true);
    overheadLightBox->setValue(intensity);
    overheadLightBox->blockSignals(false);
    realtime->setOverheadLightIntensity(intensity);
}

void MainWindow::onOverheadLightBoxChanged(double value) {
    int sliderValue = static_cast<int>(value * 100.0);
    overheadLightSlider->blockSignals(true);
    overheadLightSlider->setValue(sliderValue);
    overheadLightSlider->blockSignals(false);
    realtime->setOverheadLightIntensity(value);
}

void MainWindow::onPlayerColorChanged() {
    QColor currentColor = QColor(255, 255, 255);
    QColor newColor = QColorDialog::getColor(currentColor, this, "Select Player Light Color");
    if (newColor.isValid()) {
        float r = newColor.redF();
        float g = newColor.greenF();
        float b = newColor.blueF();
        realtime->setPlayerLightColor(r, g, b);
        
        QString colorName = newColor.name();
        playerColorButton->setText(colorName.toUpper());
        playerColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid black;").arg(colorName));
    }
}
