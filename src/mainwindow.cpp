// Defined before including Qt/OpenGL to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

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
#include <QButtonGroup>
#include <QDialog>
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
    
    // Create L-system tab
    lSystemTab = new QWidget();
    QHBoxLayout *lSystemMainLayout = new QHBoxLayout();
    
    QWidget *lSystemControls = new QWidget();
    QVBoxLayout *lSystemControlsLayout = new QVBoxLayout();
    lSystemControlsLayout->setAlignment(Qt::AlignTop);
    
    QScrollArea *lSystemScrollArea = new QScrollArea();
    lSystemScrollArea->setWidget(lSystemControls);
    lSystemScrollArea->setWidgetResizable(true);
    lSystemScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lSystemScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    lSystemScrollArea->setMinimumWidth(280);
    lSystemScrollArea->setMaximumWidth(300);
    lSystemScrollArea->setMaximumHeight(500);
    
    lSystemWidget = new LSystemWidget();
    
    QLabel *treeTypeLabel = new QLabel("Tree Type:");
    QComboBox *treeTypeComboBox = new QComboBox();
    treeTypeComboBox->addItem("Dead Pine");
    treeTypeComboBox->addItem("Dead Oak");
    treeTypeComboBox->addItem("Dead Birch");
    treeTypeComboBox->setCurrentIndex(0);
    
    QGroupBox *treeTypeLayout = new QGroupBox();
    QHBoxLayout *treeTypeLayout_inner = new QHBoxLayout();
    treeTypeLayout_inner->addWidget(treeTypeLabel);
    treeTypeLayout_inner->addWidget(treeTypeComboBox);
    treeTypeLayout->setLayout(treeTypeLayout_inner);
    
    QLabel *trunkLengthLabel = new QLabel("Trunk Length:");
    QSlider *trunkLengthSlider = new QSlider(Qt::Orientation::Horizontal);
    trunkLengthSlider->setMinimum(10);
    trunkLengthSlider->setMaximum(200);
    trunkLengthSlider->setValue(100);
    trunkLengthSlider->setMaximumWidth(160);
    QDoubleSpinBox *trunkLengthBox = new QDoubleSpinBox();
    trunkLengthBox->setMinimum(0.1);
    trunkLengthBox->setMaximum(2.0);
    trunkLengthBox->setSingleStep(0.1);
    trunkLengthBox->setDecimals(2);
    trunkLengthBox->setValue(1.0);
    trunkLengthBox->setFixedWidth(70);
    
    QLabel *trunkRadiusLabel = new QLabel("Trunk Radius:");
    QSlider *trunkRadiusSlider = new QSlider(Qt::Orientation::Horizontal);
    trunkRadiusSlider->setMinimum(1);
    trunkRadiusSlider->setMaximum(15);
    trunkRadiusSlider->setValue(5);
    trunkRadiusSlider->setMaximumWidth(160);
    QDoubleSpinBox *trunkRadiusBox = new QDoubleSpinBox();
    trunkRadiusBox->setMinimum(0.01);
    trunkRadiusBox->setMaximum(0.15);
    trunkRadiusBox->setSingleStep(0.01);
    trunkRadiusBox->setDecimals(2);
    trunkRadiusBox->setValue(0.05);
    trunkRadiusBox->setFixedWidth(70);
    
    QLabel *numBranchLayersLabel = new QLabel("Branch Layers:");
    QSlider *numBranchLayersSlider = new QSlider(Qt::Orientation::Horizontal);
    numBranchLayersSlider->setMinimum(0);
    numBranchLayersSlider->setMaximum(10);
    numBranchLayersSlider->setValue(3);
    numBranchLayersSlider->setMaximumWidth(160);
    QSpinBox *numBranchLayersBox = new QSpinBox();
    numBranchLayersBox->setMinimum(0);
    numBranchLayersBox->setMaximum(10);
    numBranchLayersBox->setSingleStep(1);
    numBranchLayersBox->setValue(3);
    numBranchLayersBox->setFixedWidth(70);
    
    QLabel *branchesPerLayerLabel = new QLabel("Branches Per Layer:");
    QSlider *branchesPerLayerSlider = new QSlider(Qt::Orientation::Horizontal);
    branchesPerLayerSlider->setMinimum(1);
    branchesPerLayerSlider->setMaximum(8);
    branchesPerLayerSlider->setValue(4);
    branchesPerLayerSlider->setMaximumWidth(160);
    QSpinBox *branchesPerLayerBox = new QSpinBox();
    branchesPerLayerBox->setMinimum(1);
    branchesPerLayerBox->setMaximum(8);
    branchesPerLayerBox->setSingleStep(1);
    branchesPerLayerBox->setValue(4);
    branchesPerLayerBox->setFixedWidth(70);
    
    QLabel *branchLengthLabel = new QLabel("Branch Length:");
    QSlider *branchLengthSlider = new QSlider(Qt::Orientation::Horizontal);
    branchLengthSlider->setMinimum(10);
    branchLengthSlider->setMaximum(200);
    branchLengthSlider->setValue(30);
    branchLengthSlider->setMaximumWidth(160);
    QDoubleSpinBox *branchLengthBox = new QDoubleSpinBox();
    branchLengthBox->setMinimum(0.1);
    branchLengthBox->setMaximum(2.0);
    branchLengthBox->setSingleStep(0.1);
    branchLengthBox->setDecimals(2);
    branchLengthBox->setValue(0.3);
    branchLengthBox->setFixedWidth(70);
    
    QLabel *branchDepthLabel = new QLabel("Branch Depth:");
    QSlider *branchDepthSlider = new QSlider(Qt::Orientation::Horizontal);
    branchDepthSlider->setMinimum(0);
    branchDepthSlider->setMaximum(8);
    branchDepthSlider->setValue(2);
    branchDepthSlider->setMaximumWidth(160);
    QSpinBox *branchDepthBox = new QSpinBox();
    branchDepthBox->setMinimum(0);
    branchDepthBox->setMaximum(8);
    branchDepthBox->setSingleStep(1);
    branchDepthBox->setValue(2);
    branchDepthBox->setFixedWidth(70);
    
    QLabel *branchAngleLabel = new QLabel("Branch Angle:");
    QSlider *branchAngleSlider = new QSlider(Qt::Orientation::Horizontal);
    branchAngleSlider->setMinimum(0);
    branchAngleSlider->setMaximum(90);
    branchAngleSlider->setValue(30);
    branchAngleSlider->setMaximumWidth(160);
    QSpinBox *branchAngleBox = new QSpinBox();
    branchAngleBox->setMinimum(0);
    branchAngleBox->setMaximum(90);
    branchAngleBox->setSingleStep(5);
    branchAngleBox->setValue(30);
    branchAngleBox->setFixedWidth(70);
    
    QLabel *zoomLabel = new QLabel("Zoom:");
    QSlider *zoomSlider = new QSlider(Qt::Orientation::Horizontal);
    zoomSlider->setMinimum(10);
    zoomSlider->setMaximum(200);
    zoomSlider->setValue(100);
    zoomSlider->setMaximumWidth(160);
    QDoubleSpinBox *zoomBox = new QDoubleSpinBox();
    zoomBox->setMinimum(0.1);
    zoomBox->setMaximum(2.0);
    zoomBox->setSingleStep(0.1);
    zoomBox->setDecimals(2);
    zoomBox->setValue(1.0);
    zoomBox->setFixedWidth(70);
    
    QGroupBox *trunkLengthLayout = new QGroupBox();
    QHBoxLayout *trunkLengthLayout_inner = new QHBoxLayout();
    trunkLengthLayout_inner->addWidget(trunkLengthLabel);
    trunkLengthLayout_inner->addWidget(trunkLengthSlider);
    trunkLengthLayout_inner->addWidget(trunkLengthBox);
    trunkLengthLayout->setLayout(trunkLengthLayout_inner);
    
    QGroupBox *trunkRadiusLayout = new QGroupBox();
    QHBoxLayout *trunkRadiusLayout_inner = new QHBoxLayout();
    trunkRadiusLayout_inner->addWidget(trunkRadiusLabel);
    trunkRadiusLayout_inner->addWidget(trunkRadiusSlider);
    trunkRadiusLayout_inner->addWidget(trunkRadiusBox);
    trunkRadiusLayout->setLayout(trunkRadiusLayout_inner);
    
    QGroupBox *numBranchLayersLayout = new QGroupBox();
    QHBoxLayout *numBranchLayersLayout_inner = new QHBoxLayout();
    numBranchLayersLayout_inner->addWidget(numBranchLayersLabel);
    numBranchLayersLayout_inner->addWidget(numBranchLayersSlider);
    numBranchLayersLayout_inner->addWidget(numBranchLayersBox);
    numBranchLayersLayout->setLayout(numBranchLayersLayout_inner);
    
    QGroupBox *branchesPerLayerLayout = new QGroupBox();
    QHBoxLayout *branchesPerLayerLayout_inner = new QHBoxLayout();
    branchesPerLayerLayout_inner->addWidget(branchesPerLayerLabel);
    branchesPerLayerLayout_inner->addWidget(branchesPerLayerSlider);
    branchesPerLayerLayout_inner->addWidget(branchesPerLayerBox);
    branchesPerLayerLayout->setLayout(branchesPerLayerLayout_inner);
    
    QGroupBox *branchLengthLayout = new QGroupBox();
    QHBoxLayout *branchLengthLayout_inner = new QHBoxLayout();
    branchLengthLayout_inner->addWidget(branchLengthLabel);
    branchLengthLayout_inner->addWidget(branchLengthSlider);
    branchLengthLayout_inner->addWidget(branchLengthBox);
    branchLengthLayout->setLayout(branchLengthLayout_inner);
    
    QGroupBox *branchDepthLayout = new QGroupBox();
    QHBoxLayout *branchDepthLayout_inner = new QHBoxLayout();
    branchDepthLayout_inner->addWidget(branchDepthLabel);
    branchDepthLayout_inner->addWidget(branchDepthSlider);
    branchDepthLayout_inner->addWidget(branchDepthBox);
    branchDepthLayout->setLayout(branchDepthLayout_inner);
    
    QGroupBox *branchAngleLayout = new QGroupBox();
    QHBoxLayout *branchAngleLayout_inner = new QHBoxLayout();
    branchAngleLayout_inner->addWidget(branchAngleLabel);
    branchAngleLayout_inner->addWidget(branchAngleSlider);
    branchAngleLayout_inner->addWidget(branchAngleBox);
    branchAngleLayout->setLayout(branchAngleLayout_inner);
    
    QLabel *subBranchAngleLabel = new QLabel("Sub Branch Angle:");
    QSlider *subBranchAngleSlider = new QSlider(Qt::Orientation::Horizontal);
    subBranchAngleSlider->setMinimum(0);
    subBranchAngleSlider->setMaximum(90);
    subBranchAngleSlider->setValue(45);
    subBranchAngleSlider->setMaximumWidth(160);
    QSpinBox *subBranchAngleBox = new QSpinBox();
    subBranchAngleBox->setMinimum(0);
    subBranchAngleBox->setMaximum(90);
    subBranchAngleBox->setSingleStep(5);
    subBranchAngleBox->setValue(45);
    subBranchAngleBox->setFixedWidth(70);
    
    QGroupBox *subBranchAngleLayout = new QGroupBox();
    QHBoxLayout *subBranchAngleLayout_inner = new QHBoxLayout();
    subBranchAngleLayout_inner->addWidget(subBranchAngleLabel);
    subBranchAngleLayout_inner->addWidget(subBranchAngleSlider);
    subBranchAngleLayout_inner->addWidget(subBranchAngleBox);
    subBranchAngleLayout->setLayout(subBranchAngleLayout_inner);
    
    QGroupBox *zoomLayout = new QGroupBox();
    QHBoxLayout *zoomLayout_inner = new QHBoxLayout();
    zoomLayout_inner->addWidget(zoomLabel);
    zoomLayout_inner->addWidget(zoomSlider);
    zoomLayout_inner->addWidget(zoomBox);
    zoomLayout->setLayout(zoomLayout_inner);
    
    lSystemControlsLayout->addWidget(treeTypeLayout);
    lSystemControlsLayout->addWidget(trunkLengthLayout);
    lSystemControlsLayout->addWidget(trunkRadiusLayout);
    lSystemControlsLayout->addWidget(numBranchLayersLayout);
    lSystemControlsLayout->addWidget(branchesPerLayerLayout);
    lSystemControlsLayout->addWidget(branchLengthLayout);
    lSystemControlsLayout->addWidget(branchDepthLayout);
    lSystemControlsLayout->addWidget(branchAngleLayout);
    lSystemControlsLayout->addWidget(subBranchAngleLayout);
    lSystemControlsLayout->addWidget(zoomLayout);
    lSystemControlsLayout->addStretch();
    
    auto updateSliderVisibility = [=](TreeType treeType) {
        bool showAllSliders = true;
        trunkLengthLayout->setVisible(showAllSliders);
        trunkRadiusLayout->setVisible(showAllSliders);
        numBranchLayersLayout->setVisible(showAllSliders);
        branchesPerLayerLayout->setVisible(showAllSliders);
        branchLengthLayout->setVisible(showAllSliders);
        branchDepthLayout->setVisible(showAllSliders);
        branchAngleLayout->setVisible(showAllSliders);
        subBranchAngleLayout->setVisible(showAllSliders);
    };
    
    lSystemControls->setLayout(lSystemControlsLayout);
    
    lSystemMainLayout->addWidget(lSystemScrollArea);
    lSystemMainLayout->addWidget(lSystemWidget, 1);
    lSystemTab->setLayout(lSystemMainLayout);
    
    auto updateTree = [=]() {
        TreeParameters params;
        params.treeType = static_cast<TreeType>(treeTypeComboBox->currentIndex());
        params.trunkLength = trunkLengthBox->value();
        params.trunkRadius = trunkRadiusBox->value();
        params.branchAngle = static_cast<float>(branchAngleBox->value());
        params.subBranchAngle = static_cast<float>(subBranchAngleBox->value());
        params.branchLength = branchLengthBox->value();
        params.numBranchLayers = numBranchLayersBox->value();
        params.branchesPerLayer = branchesPerLayerBox->value();
        params.branchDepth = branchDepthBox->value();
        
        lSystemWidget->generateTree(params);
    };
    
    connect(treeTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        TreeType treeType = static_cast<TreeType>(index);
        updateSliderVisibility(treeType);
        updateTree();
    });
    
    updateSliderVisibility(TreeType::DEAD_PINE);
    
    connect(trunkLengthSlider, &QSlider::valueChanged, [=](int value) {
        double val = value / 100.0;
        trunkLengthBox->blockSignals(true);
        trunkLengthBox->setValue(val);
        trunkLengthBox->blockSignals(false);
        updateTree();
    });
    connect(trunkLengthBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        trunkLengthSlider->blockSignals(true);
        trunkLengthSlider->setValue(static_cast<int>(value * 100));
        trunkLengthSlider->blockSignals(false);
        updateTree();
    });
    
    connect(trunkRadiusSlider, &QSlider::valueChanged, [=](int value) {
        double val = value / 100.0;
        trunkRadiusBox->blockSignals(true);
        trunkRadiusBox->setValue(val);
        trunkRadiusBox->blockSignals(false);
        updateTree();
    });
    connect(trunkRadiusBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        trunkRadiusSlider->blockSignals(true);
        trunkRadiusSlider->setValue(static_cast<int>(value * 100));
        trunkRadiusSlider->blockSignals(false);
        updateTree();
    });
    
    connect(numBranchLayersSlider, &QSlider::valueChanged, [=](int value) {
        numBranchLayersBox->blockSignals(true);
        numBranchLayersBox->setValue(value);
        numBranchLayersBox->blockSignals(false);
        updateTree();
    });
    connect(numBranchLayersBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        numBranchLayersSlider->blockSignals(true);
        numBranchLayersSlider->setValue(value);
        numBranchLayersSlider->blockSignals(false);
        updateTree();
    });
    
    connect(branchesPerLayerSlider, &QSlider::valueChanged, [=](int value) {
        branchesPerLayerBox->blockSignals(true);
        branchesPerLayerBox->setValue(value);
        branchesPerLayerBox->blockSignals(false);
        updateTree();
    });
    connect(branchesPerLayerBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        branchesPerLayerSlider->blockSignals(true);
        branchesPerLayerSlider->setValue(value);
        branchesPerLayerSlider->blockSignals(false);
        updateTree();
    });
    
    connect(branchLengthSlider, &QSlider::valueChanged, [=](int value) {
        double val = value / 100.0;
        branchLengthBox->blockSignals(true);
        branchLengthBox->setValue(val);
        branchLengthBox->blockSignals(false);
        updateTree();
    });
    connect(branchLengthBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        branchLengthSlider->blockSignals(true);
        branchLengthSlider->setValue(static_cast<int>(value * 100));
        branchLengthSlider->blockSignals(false);
        updateTree();
    });
    
    connect(branchDepthSlider, &QSlider::valueChanged, [=](int value) {
        branchDepthBox->blockSignals(true);
        branchDepthBox->setValue(value);
        branchDepthBox->blockSignals(false);
        updateTree();
    });
    connect(branchDepthBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        branchDepthSlider->blockSignals(true);
        branchDepthSlider->setValue(value);
        branchDepthSlider->blockSignals(false);
        updateTree();
    });
    
    connect(branchAngleSlider, &QSlider::valueChanged, [=](int value) {
        branchAngleBox->blockSignals(true);
        branchAngleBox->setValue(value);
        branchAngleBox->blockSignals(false);
        updateTree();
    });
    connect(branchAngleBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        branchAngleSlider->blockSignals(true);
        branchAngleSlider->setValue(value);
        branchAngleSlider->blockSignals(false);
        updateTree();
    });
    
    connect(subBranchAngleSlider, &QSlider::valueChanged, [=](int value) {
        subBranchAngleBox->blockSignals(true);
        subBranchAngleBox->setValue(value);
        subBranchAngleBox->blockSignals(false);
        updateTree();
    });
    connect(subBranchAngleBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        subBranchAngleSlider->blockSignals(true);
        subBranchAngleSlider->setValue(value);
        subBranchAngleSlider->blockSignals(false);
        updateTree();
    });
    
    connect(zoomSlider, &QSlider::valueChanged, [=](int value) {
        double val = value / 100.0;
        zoomBox->blockSignals(true);
        zoomBox->setValue(val);
        zoomBox->blockSignals(false);
        lSystemWidget->setZoom(static_cast<float>(val));
    });
    connect(zoomBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        zoomSlider->blockSignals(true);
        zoomSlider->setValue(static_cast<int>(value * 100));
        zoomSlider->blockSignals(false);
        lSystemWidget->setZoom(static_cast<float>(value));
    });
    
    updateTree();

    // Add tabs
    tabWidget->addTab(renderTab, "Render");
    tabWidget->addTab(mapBuilderWidget, "Map Builder");
    tabWidget->addTab(lSystemTab, "L-System");
    
    // Create labels in sidebox
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
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
    
    fpsModeCheckbox = new QCheckBox();
    fpsModeCheckbox->setText(QStringLiteral("FPS Mode"));
    fpsModeCheckbox->setChecked(false);
    
    motionBlurCheckbox = new QCheckBox();
    motionBlurCheckbox->setText(QStringLiteral("Motion Blur"));
    motionBlurCheckbox->setChecked(false);
    
    depthVisualizationCheckbox = new QCheckBox();
    depthVisualizationCheckbox->setText(QStringLiteral("Visualize Depth"));
    depthVisualizationCheckbox->setChecked(false);
    
    QLabel *gbufferVizLabel = new QLabel("G-Buffer Visualization:");
    gbufferVizComboBox = new QComboBox();
    gbufferVizComboBox->addItem("None");
    gbufferVizComboBox->addItem("Position");
    gbufferVizComboBox->addItem("Normal");
    gbufferVizComboBox->addItem("Albedo");
    gbufferVizComboBox->addItem("Velocity");
    gbufferVizComboBox->setCurrentIndex(0);
    
    QLabel *motionBlurSamples_label = new QLabel();
    motionBlurSamples_label->setText("Motion Blur Samples:");
    
    motionBlurSamplesSlider = new QSlider(Qt::Orientation::Horizontal);
    motionBlurSamplesSlider->setTickInterval(1);
    motionBlurSamplesSlider->setMinimum(1);
    motionBlurSamplesSlider->setMaximum(32);
    motionBlurSamplesSlider->setValue(3);
    motionBlurSamplesSlider->setMaximumWidth(160);

    motionBlurSamplesBox = new QSpinBox();
    motionBlurSamplesBox->setMinimum(1);
    motionBlurSamplesBox->setMaximum(32);
    motionBlurSamplesBox->setSingleStep(1);
    motionBlurSamplesBox->setValue(3);
    motionBlurSamplesBox->setFixedWidth(70);
    
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
    
    QLabel *cameraPath_label = new QLabel();
    cameraPath_label->setText("Camera Path");
    cameraPath_label->setFont(font);
    
    addPathPointButton = new QPushButton("Add Point");
    playPathButton = new QPushButton("Play Path");
    stopPathButton = new QPushButton("Stop Path");
    clearPathButton = new QPushButton("Clear Path");
    
    stopPathButton->setEnabled(false);
    
    QLabel *pathDuration_label = new QLabel("Path Duration (seconds):");
    pathDurationSlider = new QSlider(Qt::Orientation::Horizontal);
    pathDurationSlider->setMinimum(1);
    pathDurationSlider->setMaximum(120);
    pathDurationSlider->setValue(10);
    pathDurationSlider->setMaximumWidth(160);
    pathDurationBox = new QDoubleSpinBox();
    pathDurationBox->setMinimum(1.0);
    pathDurationBox->setMaximum(120.0);
    pathDurationBox->setSingleStep(1.0);
    pathDurationBox->setDecimals(1);
    pathDurationBox->setValue(10.0);
    pathDurationBox->setFixedWidth(70);
    
    QLabel *enemy_label = new QLabel();
    enemy_label->setText("Enemies");
    enemy_label->setFont(font);
    
    enemyAutoSpawnCheckbox = new QCheckBox();
    enemyAutoSpawnCheckbox->setText("Auto Spawn");
    enemyAutoSpawnCheckbox->setChecked(false);
    
    QLabel *enemySpawnDelay_label = new QLabel("Spawn Delay (seconds):");
    enemySpawnDelaySlider = new QSlider(Qt::Orientation::Horizontal);
    enemySpawnDelaySlider->setMinimum(1);
    enemySpawnDelaySlider->setMaximum(60);
    enemySpawnDelaySlider->setValue(10);
    enemySpawnDelaySlider->setMaximumWidth(160);
    enemySpawnDelayBox = new QDoubleSpinBox();
    enemySpawnDelayBox->setMinimum(1.0);
    enemySpawnDelayBox->setMaximum(60.0);
    enemySpawnDelayBox->setSingleStep(1.0);
    enemySpawnDelayBox->setDecimals(1);
    enemySpawnDelayBox->setValue(10.0);
    enemySpawnDelayBox->setFixedWidth(70);
    
    enemyManualSpawnButton = new QPushButton("Spawn Enemy");
    
    QLabel *playerColor_label = new QLabel();
    playerColor_label->setText("Player Light Color:");
    
    playerColorButton = new QPushButton();
    playerColorButton->setText("White");
    playerColorButton->setStyleSheet("background-color: white; border: 1px solid black;");
    playerColorButton->setMinimumHeight(30);

    QLabel *ec_label = new QLabel();
    ec_label->setText("Extra Credit");
    ec_label->setFont(font);
    QLabel *near_label = new QLabel();
    near_label->setText("Near Plane:");
    QLabel *far_label = new QLabel();
    far_label->setText("Far Plane:");

    // Create file uploader for scene file
    uploadFile = new QPushButton();
    uploadFile->setText(QStringLiteral("Upload Scene File"));
    
    saveImage = new QPushButton();
    saveImage->setText(QStringLiteral("Save Image"));

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
    farSlider->setMaximum(50000);
    farSlider->setValue(5000);
    farSlider->setMaximumWidth(160);

    farBox = new QDoubleSpinBox();
    farBox->setMinimum(10.f);
    farBox->setMaximum(500.f);
    farBox->setSingleStep(0.1f);
    farBox->setValue(50.f);
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
    vLayout->addWidget(fpsModeCheckbox);
    vLayout->addWidget(motionBlurCheckbox);
    vLayout->addWidget(depthVisualizationCheckbox);
    
    QGroupBox *gbufferVizLayout = new QGroupBox();
    QHBoxLayout *gbufferVizLayout_inner = new QHBoxLayout();
    gbufferVizLayout_inner->addWidget(gbufferVizLabel);
    gbufferVizLayout_inner->addWidget(gbufferVizComboBox);
    gbufferVizLayout->setLayout(gbufferVizLayout_inner);
    vLayout->addWidget(gbufferVizLayout);
    
    QGroupBox *motionBlurSamplesLayout = new QGroupBox();
    QHBoxLayout *motionBlurSamplesLayout_inner = new QHBoxLayout();
    motionBlurSamplesLayout_inner->addWidget(motionBlurSamples_label);
    motionBlurSamplesLayout_inner->addWidget(motionBlurSamplesSlider);
    motionBlurSamplesLayout_inner->addWidget(motionBlurSamplesBox);
    motionBlurSamplesLayout->setLayout(motionBlurSamplesLayout_inner);
    vLayout->addWidget(motionBlurSamplesLayout);
    
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
    
    vLayout->addWidget(cameraPath_label);
    QGroupBox *cameraPathLayout = new QGroupBox();
    QVBoxLayout *pathLayout = new QVBoxLayout();
    pathLayout->addWidget(addPathPointButton);
    pathLayout->addWidget(playPathButton);
    pathLayout->addWidget(stopPathButton);
    pathLayout->addWidget(clearPathButton);
    cameraPathLayout->setLayout(pathLayout);
    vLayout->addWidget(cameraPathLayout);
    
    QGroupBox *pathDurationLayout = new QGroupBox();
    QHBoxLayout *pathDurationLayout_inner = new QHBoxLayout();
    pathDurationLayout_inner->addWidget(pathDuration_label);
    pathDurationLayout_inner->addWidget(pathDurationSlider);
    pathDurationLayout_inner->addWidget(pathDurationBox);
    pathDurationLayout->setLayout(pathDurationLayout_inner);
    vLayout->addWidget(pathDurationLayout);
    
    vLayout->addWidget(enemy_label);
    vLayout->addWidget(enemyAutoSpawnCheckbox);
    
    QGroupBox *enemySpawnDelayLayout = new QGroupBox();
    QHBoxLayout *enemySpawnDelayLayout_inner = new QHBoxLayout();
    enemySpawnDelayLayout_inner->addWidget(enemySpawnDelay_label);
    enemySpawnDelayLayout_inner->addWidget(enemySpawnDelaySlider);
    enemySpawnDelayLayout_inner->addWidget(enemySpawnDelayBox);
    enemySpawnDelayLayout->setLayout(enemySpawnDelayLayout_inner);
    vLayout->addWidget(enemySpawnDelayLayout);
    
    vLayout->addWidget(enemyManualSpawnButton);
    
    fogCheckbox = new QCheckBox();
    fogCheckbox->setText("Fog");
    fogCheckbox->setChecked(true);
    vLayout->addWidget(fogCheckbox);
    
    fogColorButton = new QPushButton();
    fogColorButton->setText("Fog Color");
    fogColorButton->setStyleSheet("background-color: rgb(13, 13, 38);");
    vLayout->addWidget(fogColorButton);
    
    QHBoxLayout *fogIntensityLayout = new QHBoxLayout();
    QLabel *fogIntensityLabel = new QLabel("Fog Intensity:");
    fogIntensitySlider = new QSlider(Qt::Horizontal);
    fogIntensitySlider->setMinimum(1);
    fogIntensitySlider->setMaximum(100);
    fogIntensitySlider->setValue(1);
    fogIntensitySlider->setMaximumWidth(160);
    fogIntensityBox = new QDoubleSpinBox();
    fogIntensityBox->setMinimum(0.1);
    fogIntensityBox->setMaximum(1.0);
    fogIntensityBox->setSingleStep(0.01);
    fogIntensityBox->setValue(1.0);
    fogIntensityBox->setFixedWidth(70);
    fogIntensityLayout->addWidget(fogIntensityLabel);
    fogIntensityLayout->addWidget(fogIntensitySlider);
    fogIntensityLayout->addWidget(fogIntensityBox);
    vLayout->addLayout(fogIntensityLayout);
    
    flashlightCheckbox = new QCheckBox();
    flashlightCheckbox->setText("Flashlight");
    flashlightCheckbox->setChecked(false);
    vLayout->addWidget(flashlightCheckbox);
    
    // Flashlight charge label
    flashlightChargeLabel = new QLabel();
    flashlightChargeLabel->setText("Flashlight Charge: 100%");
    vLayout->addWidget(flashlightChargeLabel);
    
    teleportButton = new QPushButton();
    teleportButton->setText("Teleport to (0,0,0)");
    vLayout->addWidget(teleportButton);
    
    // Post-processing filters
    QLabel *filter_label = new QLabel();
    filter_label->setText("Post-Processing Filters");
    filter_label->setFont(font);
    vLayout->addWidget(filter_label);
    
    filterGroupBox = new QGroupBox();
    QVBoxLayout *filterLayout = new QVBoxLayout();
    filterButtonGroup = new QButtonGroup(this);
    filterButtonGroup->setExclusive(true);
    
    filterNoneCheckbox = new QCheckBox("None");
    filterNoneCheckbox->setChecked(true);
    filterWaveCheckbox = new QCheckBox("Wave");
    filterShakeCheckbox = new QCheckBox("Shake");
    filterInkOutlineCheckbox = new QCheckBox("Ink Outline");
    filterCartoonCheckbox = new QCheckBox("Cartoon");
    filterStaticNoiseCheckbox = new QCheckBox("Static Noise");
    filterDepthCheckbox = new QCheckBox("Depth");
    filterColorGradingCheckbox = new QCheckBox("Color Grading");
    filterPixelateCheckbox = new QCheckBox("Pixelate");
    
    filterButtonGroup->addButton(filterNoneCheckbox, 0);
    filterButtonGroup->addButton(filterWaveCheckbox, 1);
    filterButtonGroup->addButton(filterShakeCheckbox, 2);
    filterButtonGroup->addButton(filterInkOutlineCheckbox, 3);
    filterButtonGroup->addButton(filterCartoonCheckbox, 4);
    filterButtonGroup->addButton(filterStaticNoiseCheckbox, 5);
    filterButtonGroup->addButton(filterDepthCheckbox, 6);
    filterButtonGroup->addButton(filterColorGradingCheckbox, 7);
    
    filterLayout->addWidget(filterNoneCheckbox);
    filterLayout->addWidget(filterWaveCheckbox);
    filterLayout->addWidget(filterShakeCheckbox);
    filterLayout->addWidget(filterInkOutlineCheckbox);
    filterLayout->addWidget(filterCartoonCheckbox);
    filterLayout->addWidget(filterStaticNoiseCheckbox);
    filterLayout->addWidget(filterDepthCheckbox);
    filterColorGradingCheckbox->setChecked(true);
    filterLayout->addWidget(filterColorGradingCheckbox);
    
    lutSelectButton = new QPushButton("Select LUT");
    lutSelectButton->setEnabled(true);
    filterLayout->addWidget(lutSelectButton);
    
    filterGroupBox->setLayout(filterLayout);
    vLayout->addWidget(filterGroupBox);
    
    grainOverlayCheckbox = new QCheckBox("Grain Overlay");
    grainOverlayCheckbox->setChecked(false);
    vLayout->addWidget(grainOverlayCheckbox);
    
    QGroupBox *grainOpacityGroup = new QGroupBox("Grain Opacity");
    QHBoxLayout *grainOpacityLayout = new QHBoxLayout();
    grainOpacitySlider = new QSlider(Qt::Orientation::Horizontal);
    grainOpacitySlider->setMinimum(0);
    grainOpacitySlider->setMaximum(100);
    grainOpacitySlider->setValue(100);
    grainOpacityBox = new QDoubleSpinBox();
    grainOpacityBox->setMinimum(0.0);
    grainOpacityBox->setMaximum(1.0);
    grainOpacityBox->setSingleStep(0.01);
    grainOpacityBox->setDecimals(2);
    grainOpacityBox->setValue(1.0);
    grainOpacityBox->setFixedWidth(70);
    grainOpacityLayout->addWidget(grainOpacitySlider);
    grainOpacityLayout->addWidget(grainOpacityBox);
    grainOpacityGroup->setLayout(grainOpacityLayout);
    vLayout->addWidget(grainOpacityGroup);
    
    filterPixelateCheckbox->setChecked(true);
    vLayout->addWidget(filterPixelateCheckbox);
    
    // Particle system
    QLabel *particle_label = new QLabel();
    particle_label->setText("Particles");
    particle_label->setFont(font);
    vLayout->addWidget(particle_label);
    
    particleGroupBox = new QGroupBox();
    QVBoxLayout *particleLayout = new QVBoxLayout();
    
    particlesEnabledCheckbox = new QCheckBox("Enable Particles");
    particlesEnabledCheckbox->setChecked(true);
    particleLayout->addWidget(particlesEnabledCheckbox);
    
    dirtParticlesCheckbox = new QCheckBox("Dirt Particles");
    dirtParticlesCheckbox->setChecked(true);
    particleLayout->addWidget(dirtParticlesCheckbox);
    
    fogWispsCheckbox = new QCheckBox("Fog Wisps");
    fogWispsCheckbox->setChecked(true);
    particleLayout->addWidget(fogWispsCheckbox);
    
    QLabel *dirtSpawnRate_label = new QLabel("Dirt Spawn Rate (per second):");
    dirtSpawnRateSlider = new QSlider(Qt::Horizontal);
    dirtSpawnRateSlider->setMinimum(1);
    dirtSpawnRateSlider->setMaximum(50);
    dirtSpawnRateSlider->setValue(10);
    dirtSpawnRateSlider->setMaximumWidth(160);
    dirtSpawnRateBox = new QDoubleSpinBox();
    dirtSpawnRateBox->setMinimum(1.0);
    dirtSpawnRateBox->setMaximum(50.0);
    dirtSpawnRateBox->setSingleStep(1.0);
    dirtSpawnRateBox->setDecimals(1);
    dirtSpawnRateBox->setValue(10.0);
    dirtSpawnRateBox->setFixedWidth(70);
    QHBoxLayout *dirtSpawnRateLayout = new QHBoxLayout();
    dirtSpawnRateLayout->addWidget(dirtSpawnRateSlider);
    dirtSpawnRateLayout->addWidget(dirtSpawnRateBox);
    particleLayout->addWidget(dirtSpawnRate_label);
    particleLayout->addLayout(dirtSpawnRateLayout);
    
    QLabel *fogWispInterval_label = new QLabel("Fog Wisp Interval (seconds):");
    fogWispIntervalSlider = new QSlider(Qt::Horizontal);
    fogWispIntervalSlider->setMinimum(1);
    fogWispIntervalSlider->setMaximum(100);
    fogWispIntervalSlider->setValue(15); // 1.5 seconds
    fogWispIntervalSlider->setMaximumWidth(160);
    fogWispIntervalBox = new QDoubleSpinBox();
    fogWispIntervalBox->setMinimum(0.1);
    fogWispIntervalBox->setMaximum(10.0);
    fogWispIntervalBox->setSingleStep(0.1);
    fogWispIntervalBox->setDecimals(1);
    fogWispIntervalBox->setValue(1.5);
    fogWispIntervalBox->setFixedWidth(70);
    QHBoxLayout *fogWispIntervalLayout = new QHBoxLayout();
    fogWispIntervalLayout->addWidget(fogWispIntervalSlider);
    fogWispIntervalLayout->addWidget(fogWispIntervalBox);
    particleLayout->addWidget(fogWispInterval_label);
    particleLayout->addLayout(fogWispIntervalLayout);
    
    QLabel *maxParticles_label = new QLabel("Max Particles:");
    maxParticlesSlider = new QSlider(Qt::Horizontal);
    maxParticlesSlider->setMinimum(50);
    maxParticlesSlider->setMaximum(1000);
    maxParticlesSlider->setValue(200);
    maxParticlesSlider->setMaximumWidth(160);
    maxParticlesBox = new QSpinBox();
    maxParticlesBox->setMinimum(50);
    maxParticlesBox->setMaximum(1000);
    maxParticlesBox->setSingleStep(50);
    maxParticlesBox->setValue(200);
    maxParticlesBox->setFixedWidth(70);
    QHBoxLayout *maxParticlesLayout = new QHBoxLayout();
    maxParticlesLayout->addWidget(maxParticlesSlider);
    maxParticlesLayout->addWidget(maxParticlesBox);
    particleLayout->addWidget(maxParticles_label);
    particleLayout->addLayout(maxParticlesLayout);
    
    particleGroupBox->setLayout(particleLayout);
    vLayout->addWidget(particleGroupBox);

    // Extra Credit:
    vLayout->addWidget(ec_label);
    vLayout->addWidget(ec1);
    vLayout->addWidget(ec2);
    vLayout->addWidget(ec3);
    vLayout->addWidget(ec4);

    connectUIElements();

    // Set default values for near and far planes
    onValChangeNearBox(0.1f);
    onValChangeFarBox(50.f);
    
    // Initialize enemy spawn delay
    realtime->getEnemyManager().setSpawnDelay(10.0f);
}

void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

void MainWindow::connectUIElements() {
    //connectPerPixelFilter();
    //connectKernelBasedFilter();
    connectUploadFile();
    connectSaveImage();
    connectNear();
    connectFar();
    connectExtraCredit();
    
    connect(realtime, &Realtime::telemetryUpdate, this, &MainWindow::onTelemetryUpdate);
    
    // Connect camera controls
    connect(flyingModeCheckbox, &QCheckBox::clicked, this, &MainWindow::onFlyingModeChanged);
    connect(fpsModeCheckbox, &QCheckBox::clicked, this, &MainWindow::onFpsModeChanged);
    connect(motionBlurCheckbox, &QCheckBox::clicked, this, &MainWindow::onMotionBlurChanged);
    connect(depthVisualizationCheckbox, &QCheckBox::clicked, this, &MainWindow::onDepthVisualizationChanged);
    connect(gbufferVizComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onGBufferVizChanged);
    connect(motionBlurSamplesSlider, &QSlider::valueChanged, this, &MainWindow::onMotionBlurSamplesChanged);
    connect(motionBlurSamplesBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMotionBlurSamplesBoxChanged);
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
    
    connect(addPathPointButton, &QPushButton::clicked, this, &MainWindow::onAddPathPoint);
    connect(playPathButton, &QPushButton::clicked, this, &MainWindow::onPlayPath);
    connect(stopPathButton, &QPushButton::clicked, this, &MainWindow::onStopPath);
    connect(clearPathButton, &QPushButton::clicked, this, &MainWindow::onClearPath);
    connect(realtime, &Realtime::pathPlaybackFinished, this, &MainWindow::onStopPath);
    connect(realtime, &Realtime::fpsModeToggled, this, [this](bool enabled) {
        fpsModeCheckbox->blockSignals(true);
        fpsModeCheckbox->setChecked(enabled);
        fpsModeCheckbox->blockSignals(false);
    });
    connect(pathDurationSlider, &QSlider::valueChanged, this, &MainWindow::onPathDurationChanged);
    connect(pathDurationBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onPathDurationBoxChanged);
    
    connect(enemyAutoSpawnCheckbox, &QCheckBox::checkStateChanged, this, &MainWindow::onEnemyAutoSpawnChanged);
    connect(enemySpawnDelaySlider, &QSlider::valueChanged, this, &MainWindow::onEnemySpawnDelayChanged);
    connect(enemySpawnDelayBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onEnemySpawnDelayBoxChanged);
    connect(enemyManualSpawnButton, &QPushButton::clicked, this, &MainWindow::onEnemyManualSpawn);
    
    connect(fogCheckbox, &QCheckBox::clicked, this, &MainWindow::onFogChanged);
    connect(fogColorButton, &QPushButton::clicked, this, &MainWindow::onFogColorChanged);
    connect(fogIntensitySlider, &QSlider::valueChanged, this, &MainWindow::onFogIntensityChanged);
    connect(fogIntensityBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onFogIntensityBoxChanged);
    connect(flashlightCheckbox, &QCheckBox::clicked, this, &MainWindow::onFlashlightChanged);
    connect(realtime, &Realtime::flashlightChargeChanged, this, &MainWindow::onFlashlightChargeChanged);
    connect(realtime, &Realtime::motionBlurToggled, this, [this](bool enabled) {
        motionBlurCheckbox->blockSignals(true);
        motionBlurCheckbox->setChecked(enabled);
        motionBlurCheckbox->blockSignals(false);
    });
    connect(teleportButton, &QPushButton::clicked, this, &MainWindow::onTeleportToOrigin);
    
    // Post-processing filters
    connect(filterNoneCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterNoneChanged);
    connect(filterWaveCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterWaveChanged);
    connect(filterShakeCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterShakeChanged);
    connect(filterInkOutlineCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterInkOutlineChanged);
    connect(filterCartoonCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterCartoonChanged);
    connect(filterStaticNoiseCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterStaticNoiseChanged);
    connect(filterDepthCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterDepthChanged);
    connect(filterColorGradingCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterColorGradingChanged);
    connect(filterPixelateCheckbox, &QCheckBox::clicked, this, &MainWindow::onFilterPixelateChanged);
    connect(grainOverlayCheckbox, &QCheckBox::clicked, this, &MainWindow::onGrainOverlayChanged);
    connect(grainOpacitySlider, &QSlider::valueChanged, this, [this](int value) {
        grainOpacityBox->setValue(value / 100.0);
        onGrainOpacityChanged();
    });
    connect(grainOpacityBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        grainOpacitySlider->setValue(static_cast<int>(value * 100));
        onGrainOpacityChanged();
    });
    connect(lutSelectButton, &QPushButton::clicked, this, &MainWindow::onLUTSelect);
    
    // Particle system
    connect(particlesEnabledCheckbox, &QCheckBox::clicked, this, &MainWindow::onParticlesEnabledChanged);
    connect(dirtParticlesCheckbox, &QCheckBox::clicked, this, &MainWindow::onDirtParticlesChanged);
    connect(fogWispsCheckbox, &QCheckBox::clicked, this, &MainWindow::onFogWispsChanged);
    connect(dirtSpawnRateSlider, &QSlider::valueChanged, this, &MainWindow::onDirtSpawnRateChanged);
    connect(dirtSpawnRateBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onDirtSpawnRateBoxChanged);
    connect(fogWispIntervalSlider, &QSlider::valueChanged, this, &MainWindow::onFogWispIntervalChanged);
    connect(fogWispIntervalBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onFogWispIntervalBoxChanged);
    connect(maxParticlesSlider, &QSlider::valueChanged, this, &MainWindow::onMaxParticlesChanged);
    connect(maxParticlesBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMaxParticlesBoxChanged);
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

void MainWindow::onFpsModeChanged() {
    bool enabled = fpsModeCheckbox->isChecked();
    realtime->setFpsMode(enabled);
}

void MainWindow::onMotionBlurChanged() {
    bool enabled = motionBlurCheckbox->isChecked();
    realtime->setMotionBlurEnabled(enabled);
}

void MainWindow::onMotionBlurSamplesChanged(int value) {
    motionBlurSamplesBox->blockSignals(true);
    motionBlurSamplesBox->setValue(value);
    motionBlurSamplesBox->blockSignals(false);
    realtime->setMotionBlurSamples(value);
}

void MainWindow::onMotionBlurSamplesBoxChanged(int value) {
    motionBlurSamplesSlider->blockSignals(true);
    motionBlurSamplesSlider->setValue(value);
    motionBlurSamplesSlider->blockSignals(false);
    realtime->setMotionBlurSamples(value);
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

void MainWindow::onAddPathPoint() {
    realtime->addPathWaypoint();
}

void MainWindow::onPlayPath() {
    if (realtime->getPathWaypointCount() >= 2) {
        realtime->startPathPlayback();
        playPathButton->setEnabled(false);
        stopPathButton->setEnabled(true);
        addPathPointButton->setEnabled(false);
        clearPathButton->setEnabled(false);
    }
}

void MainWindow::onStopPath() {
    realtime->stopPathPlayback();
    playPathButton->setEnabled(true);
    stopPathButton->setEnabled(false);
    addPathPointButton->setEnabled(true);
    clearPathButton->setEnabled(true);
}

void MainWindow::onClearPath() {
    realtime->clearPath();
    playPathButton->setEnabled(true);
    stopPathButton->setEnabled(false);
}

void MainWindow::onPathDurationChanged(int value) {
    double duration = static_cast<double>(value);
    pathDurationBox->blockSignals(true);
    pathDurationBox->setValue(duration);
    pathDurationBox->blockSignals(false);
    realtime->setPathDuration(duration);
}

void MainWindow::onPathDurationBoxChanged(double value) {
    pathDurationSlider->blockSignals(true);
    pathDurationSlider->setValue(static_cast<int>(value));
    pathDurationSlider->blockSignals(false);
    realtime->setPathDuration(value);
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

void MainWindow::onDepthVisualizationChanged() {
    bool enabled = depthVisualizationCheckbox->isChecked();
    realtime->setDepthVisualizationEnabled(enabled);
    if (enabled) {
        motionBlurCheckbox->setChecked(false);
        gbufferVizComboBox->setCurrentIndex(0);
    }
}

void MainWindow::onGBufferVizChanged(int index) {
    realtime->setGBufferVisualizationMode(index);
    if (index != 0) {
        motionBlurCheckbox->setChecked(false);
        depthVisualizationCheckbox->setChecked(false);
    }
}

void MainWindow::onEnemyAutoSpawnChanged(int state) {
    bool enabled = (state == Qt::Checked);
    realtime->getEnemyManager().setAutoSpawnEnabled(enabled);
}

void MainWindow::onEnemySpawnDelayChanged(int value) {
    double delay = static_cast<double>(value);
    enemySpawnDelayBox->blockSignals(true);
    enemySpawnDelayBox->setValue(delay);
    enemySpawnDelayBox->blockSignals(false);
    realtime->getEnemyManager().setSpawnDelay(delay);
}

void MainWindow::onEnemySpawnDelayBoxChanged(double value) {
    enemySpawnDelaySlider->blockSignals(true);
    enemySpawnDelaySlider->setValue(static_cast<int>(value));
    enemySpawnDelaySlider->blockSignals(false);
    realtime->getEnemyManager().setSpawnDelay(value);
}

void MainWindow::onEnemyManualSpawn() {
    glm::vec3 cameraPos = realtime->getCamera().getPosition();
    realtime->getEnemyManager().spawnEnemy(cameraPos + glm::vec3(0.0f, 50.0f, 0.0f));
}

void MainWindow::onFogChanged() {
    realtime->setFogEnabled(fogCheckbox->isChecked());
}

void MainWindow::onFogColorChanged() {
    QColor currentColor = QColor(static_cast<int>(115), static_cast<int>(122), static_cast<int>(128));
    QColor color = QColorDialog::getColor(currentColor, this, "Select Fog Color");
    if (color.isValid()) {
        realtime->setFogColor(color.redF(), color.greenF(), color.blueF());
        fogColorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                                      .arg(color.red())
                                      .arg(color.green())
                                      .arg(color.blue()));
    }
}

void MainWindow::onFogIntensityChanged(int value) {
    double intensity = 1.0 - (value - 1.0) * 0.9 / 99.0;
    intensity = std::max(0.1, std::min(1.0, intensity));
    fogIntensityBox->blockSignals(true);
    fogIntensityBox->setValue(intensity);
    fogIntensityBox->blockSignals(false);
    realtime->setFogIntensity(static_cast<float>(intensity));
}

void MainWindow::onFogIntensityBoxChanged(double value) {

    int sliderValue = static_cast<int>(1.0 + (1.0 - value) * 99.0 / 0.9);
    sliderValue = std::max(1, std::min(100, sliderValue));
    fogIntensitySlider->blockSignals(true);
    fogIntensitySlider->setValue(sliderValue);
    fogIntensitySlider->blockSignals(false);
    realtime->setFogIntensity(static_cast<float>(value));
}

void MainWindow::onFlashlightChanged() {
    realtime->setFlashlightEnabled(flashlightCheckbox->isChecked());
}

void MainWindow::onFlashlightChargeChanged(float charge, bool inPenalty) {
    QString chargeText;
    if (inPenalty) {
        float penaltyTime = realtime->getFlashlightPenaltyTimer();
        chargeText = QString("Flashlight Charge: %1% (Recharging in %2s)")
            .arg(static_cast<int>(charge))
            .arg(static_cast<int>(penaltyTime + 0.5f));
    } else {
        chargeText = QString("Flashlight Charge: %1%")
            .arg(static_cast<int>(charge));
    }
    flashlightChargeLabel->setText(chargeText);
    
    bool wasBlocked = flashlightCheckbox->blockSignals(true);
    flashlightCheckbox->setChecked(realtime->isFlashlightEnabled());
    flashlightCheckbox->blockSignals(wasBlocked);
}

void MainWindow::onTeleportToOrigin() {
    realtime->teleportToOrigin();
}

// Post-processing filter slots
void MainWindow::onFilterNoneChanged() {
    if (filterNoneCheckbox->isChecked()) {
        realtime->setFilterMode(0);
        lutSelectButton->setEnabled(false);
    }
}

void MainWindow::onFilterWaveChanged() {
    if (filterWaveCheckbox->isChecked()) {
        realtime->setFilterMode(1);
    }
}

void MainWindow::onFilterShakeChanged() {
    if (filterShakeCheckbox->isChecked()) {
        realtime->setFilterMode(2);
    }
}

void MainWindow::onFilterInkOutlineChanged() {
    if (filterInkOutlineCheckbox->isChecked()) {
        realtime->setFilterMode(3);
    }
}

void MainWindow::onFilterCartoonChanged() {
    if (filterCartoonCheckbox->isChecked()) {
        realtime->setFilterMode(4);
    }
}

void MainWindow::onFilterStaticNoiseChanged() {
    if (filterStaticNoiseCheckbox->isChecked()) {
        realtime->setFilterMode(5);
    }
}

void MainWindow::onFilterDepthChanged() {
    if (filterDepthCheckbox->isChecked()) {
        realtime->setFilterMode(6);
    }
}

void MainWindow::onFilterColorGradingChanged() {
    if (filterColorGradingCheckbox->isChecked()) {
        realtime->setFilterMode(7);
        lutSelectButton->setEnabled(true);
        onLUTSelect();
    } else {
        lutSelectButton->setEnabled(false);
    }
}

void MainWindow::onFilterPixelateChanged() {
    realtime->setPixelateEnabled(filterPixelateCheckbox->isChecked());
}

void MainWindow::onGrainOverlayChanged() {
    realtime->setGrainOverlayEnabled(grainOverlayCheckbox->isChecked());
}

void MainWindow::onGrainOpacityChanged() {
    float opacity = static_cast<float>(grainOpacityBox->value());
    realtime->setGrainOpacity(opacity);
}

void MainWindow::onLUTSelect() {
    QDialog dialog(this);
    dialog.setWindowTitle("Choose Color Grading LUT");
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QComboBox* combo = new QComboBox(&dialog);
    combo->addItem("Neutral");
    combo->addItem("Fancy Blue");
    combo->addItem("Warm");
    combo->addItem("Black & White");
    combo->addItem("Cool");
    combo->addItem("Green");
    combo->addItem("Blue");
    combo->addItem("Horror");
    combo->setCurrentIndex(realtime->getLUTChoice());
    layout->addWidget(combo);
    
    QPushButton* ok = new QPushButton("Apply", &dialog);
    layout->addWidget(ok);
    
    connect(ok, &QPushButton::clicked, [&]() {
        realtime->setLUTChoice(combo->currentIndex());
        dialog.accept();
    });
    
    dialog.exec();
}

void MainWindow::onParticlesEnabledChanged() {
    realtime->setParticlesEnabled(particlesEnabledCheckbox->isChecked());
}

void MainWindow::onDirtParticlesChanged() {
    realtime->setDirtParticlesEnabled(dirtParticlesCheckbox->isChecked());
}

void MainWindow::onFogWispsChanged() {
    realtime->setFogWispsEnabled(fogWispsCheckbox->isChecked());
}

void MainWindow::onDirtSpawnRateChanged(int value) {
    double doubleValue = static_cast<double>(value);
    dirtSpawnRateBox->blockSignals(true);
    dirtSpawnRateBox->setValue(doubleValue);
    dirtSpawnRateBox->blockSignals(false);
    realtime->setDirtSpawnRate(static_cast<float>(doubleValue));
}

void MainWindow::onDirtSpawnRateBoxChanged(double value) {
    int sliderValue = static_cast<int>(value);
    dirtSpawnRateSlider->blockSignals(true);
    dirtSpawnRateSlider->setValue(sliderValue);
    dirtSpawnRateSlider->blockSignals(false);
    realtime->setDirtSpawnRate(static_cast<float>(value));
}

void MainWindow::onFogWispIntervalChanged(int value) {
    double doubleValue = static_cast<double>(value) / 10.0;
    fogWispIntervalBox->blockSignals(true);
    fogWispIntervalBox->setValue(doubleValue);
    fogWispIntervalBox->blockSignals(false);
    realtime->setFogWispSpawnInterval(static_cast<float>(doubleValue));
}

void MainWindow::onFogWispIntervalBoxChanged(double value) {
    int sliderValue = static_cast<int>(value * 10.0);
    fogWispIntervalSlider->blockSignals(true);
    fogWispIntervalSlider->setValue(sliderValue);
    fogWispIntervalSlider->blockSignals(false);
    realtime->setFogWispSpawnInterval(static_cast<float>(value));
}

void MainWindow::onMaxParticlesChanged(int value) {
    maxParticlesBox->blockSignals(true);
    maxParticlesBox->setValue(value);
    maxParticlesBox->blockSignals(false);
    realtime->setMaxParticles(value);
}

void MainWindow::onMaxParticlesBoxChanged(int value) {
    maxParticlesSlider->blockSignals(true);
    maxParticlesSlider->setValue(value);
    maxParticlesSlider->blockSignals(false);
    realtime->setMaxParticles(value);
}
