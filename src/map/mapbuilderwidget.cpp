#include "mapbuilderwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QPixmap>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "mapproperties.h"

MapBuilderWidget::MapBuilderWidget(QWidget *parent)
    : QWidget(parent)
    , heightMapLabel(nullptr)
    , biomeMapLabel(nullptr)
    , heightMapImage(nullptr)
    , biomeMapImage(nullptr)
    , legendWidget(nullptr)
{
    //random seed generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    setupUI();
    connectControls();
    updateMapBuilderParams();
    m_mapBuilder.generateBiomeMap();
    updateVisualizations();
}


//****MOST UI STYLING WAS AI GENERATED - ChatGPT 5.1****

void MapBuilderWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controlsLayout->setAlignment(Qt::AlignTop);
    controlsLayout->setSpacing(15);
    
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    QLabel *titleLabel = new QLabel("Noise Parameters");
    titleLabel->setFont(titleFont);
    controlsLayout->addWidget(titleLabel);
    
    QGroupBox *seedGroup = new QGroupBox("Seed");
    QHBoxLayout *seedLayout = new QHBoxLayout();
    seedBox = new QSpinBox();
    seedBox->setMinimum(0);
    seedBox->setMaximum(999999);
    seedBox->setValue(0);
    randomSeedButton = new QPushButton("Random");
    randomSeedButton->setMaximumWidth(80);
    seedLayout->addWidget(seedBox);
    seedLayout->addWidget(randomSeedButton);
    seedGroup->setLayout(seedLayout);
    controlsLayout->addWidget(seedGroup);
    
    // Frequency
    QGroupBox *freqGroup = new QGroupBox("Frequency");
    QHBoxLayout *freqLayout = new QHBoxLayout();
    frequencyBox = new QDoubleSpinBox();
    frequencyBox->setMinimum(0.001);
    frequencyBox->setMaximum(1.0);
    frequencyBox->setSingleStep(0.001);
    frequencyBox->setDecimals(3);
    frequencyBox->setValue(0.01);
    freqLayout->addWidget(frequencyBox);
    freqGroup->setLayout(freqLayout);
    controlsLayout->addWidget(freqGroup);
    
    // Octaves
    QGroupBox *octavesGroup = new QGroupBox("Octaves");
    QHBoxLayout *octavesLayout = new QHBoxLayout();
    octavesBox = new QSpinBox();
    octavesBox->setMinimum(1);
    octavesBox->setMaximum(8);
    octavesBox->setValue(4);
    octavesLayout->addWidget(octavesBox);
    octavesGroup->setLayout(octavesLayout);
    controlsLayout->addWidget(octavesGroup);
    
    // Amplitude
    QGroupBox *ampGroup = new QGroupBox("Amplitude");
    QHBoxLayout *ampLayout = new QHBoxLayout();
    amplitudeBox = new QDoubleSpinBox();
    amplitudeBox->setMinimum(0.1);
    amplitudeBox->setMaximum(10.0);
    amplitudeBox->setSingleStep(0.1);
    amplitudeBox->setDecimals(1);
    amplitudeBox->setValue(1.0);
    ampLayout->addWidget(amplitudeBox);
    ampGroup->setLayout(ampLayout);
    controlsLayout->addWidget(ampGroup);
    
    // Persistence
    QGroupBox *persistGroup = new QGroupBox("Persistence");
    QHBoxLayout *persistLayout = new QHBoxLayout();
    persistenceBox = new QDoubleSpinBox();
    persistenceBox->setMinimum(0.1);
    persistenceBox->setMaximum(1.0);
    persistenceBox->setSingleStep(0.1);
    persistenceBox->setDecimals(1);
    persistenceBox->setValue(0.5);
    persistLayout->addWidget(persistenceBox);
    persistGroup->setLayout(persistLayout);
    controlsLayout->addWidget(persistGroup);
    
    // Biome Parameters
    QFont biomeFont;
    biomeFont.setPointSize(12);
    biomeFont.setBold(true);
    QLabel *biomeLabel = new QLabel("Biome Parameters");
    biomeLabel->setFont(biomeFont);
    controlsLayout->addWidget(biomeLabel);
    
    // Biome Frequency
    QGroupBox *biomeFreqGroup = new QGroupBox("Biome Frequency");
    QHBoxLayout *biomeFreqLayout = new QHBoxLayout();
    biomeFrequencyBox = new QDoubleSpinBox();
    biomeFrequencyBox->setMinimum(0.001);
    biomeFrequencyBox->setMaximum(0.1);
    biomeFrequencyBox->setSingleStep(0.001);
    biomeFrequencyBox->setDecimals(3);
    biomeFrequencyBox->setValue(0.005);
    biomeFreqLayout->addWidget(biomeFrequencyBox);
    biomeFreqGroup->setLayout(biomeFreqLayout);
    controlsLayout->addWidget(biomeFreqGroup);
    
    // Biome Octaves
    QGroupBox *biomeOctavesGroup = new QGroupBox("Biome Octaves");
    QHBoxLayout *biomeOctavesLayout = new QHBoxLayout();
    biomeOctavesBox = new QSpinBox();
    biomeOctavesBox->setMinimum(1);
    biomeOctavesBox->setMaximum(6);
    biomeOctavesBox->setValue(3);
    biomeOctavesLayout->addWidget(biomeOctavesBox);
    biomeOctavesGroup->setLayout(biomeOctavesLayout);
    controlsLayout->addWidget(biomeOctavesGroup);
    
    // Biome Warp (Domain Warping strength)
    QGroupBox *biomeWarpGroup = new QGroupBox("Biome Warp");
    QHBoxLayout *biomeWarpLayout = new QHBoxLayout();
    biomeWarpBox = new QDoubleSpinBox();
    biomeWarpBox->setMinimum(0.0);
    biomeWarpBox->setMaximum(200.0);
    biomeWarpBox->setSingleStep(10.0);
    biomeWarpBox->setDecimals(0);
    biomeWarpBox->setValue(50.0);
    biomeWarpLayout->addWidget(biomeWarpBox);
    biomeWarpGroup->setLayout(biomeWarpLayout);
    controlsLayout->addWidget(biomeWarpGroup);
    
    // Save Map Scene Button
    saveMapSceneButton = new QPushButton();
    saveMapSceneButton->setText(QStringLiteral("Save Map Scene"));
    controlsLayout->addWidget(saveMapSceneButton);
    
    // Map Dimensions
    QFont dimFont;
    dimFont.setPointSize(12);
    dimFont.setBold(true);
    QLabel *dimLabel = new QLabel("Map Dimensions");
    dimLabel->setFont(dimFont);
    controlsLayout->addWidget(dimLabel);
    
    QGroupBox *widthGroup = new QGroupBox("Width");
    QHBoxLayout *widthLayout = new QHBoxLayout();
    mapWidthBox = new QSpinBox();
    mapWidthBox->setMinimum(16);
    mapWidthBox->setMaximum(2048);
    mapWidthBox->setSingleStep(16);
    mapWidthBox->setValue(100);
    widthLayout->addWidget(mapWidthBox);
    widthGroup->setLayout(widthLayout);
    controlsLayout->addWidget(widthGroup);
    
    QGroupBox *heightGroup = new QGroupBox("Height");
    QHBoxLayout *heightLayout = new QHBoxLayout();
    mapHeightBox = new QSpinBox();
    mapHeightBox->setMinimum(16);
    mapHeightBox->setMaximum(2048);
    mapHeightBox->setSingleStep(16);
    mapHeightBox->setValue(100);
    heightLayout->addWidget(mapHeightBox);
    heightGroup->setLayout(heightLayout);
    controlsLayout->addWidget(heightGroup);
    
    controlsLayout->addStretch();
    
    // Right side: Maps display area
    QVBoxLayout *mapsLayout = new QVBoxLayout();
    
    // Maps title
    QFont mapsTitleFont;
    mapsTitleFont.setPointSize(12);
    mapsTitleFont.setBold(true);
    QLabel *mapsTitle = new QLabel("Map Views");
    mapsTitle->setFont(mapsTitleFont);
    mapsTitle->setAlignment(Qt::AlignCenter);
    mapsLayout->addWidget(mapsTitle);
    
    // Height map section
    QLabel *heightTitle = new QLabel("Height Map");
    heightTitle->setFont(dimFont);
    heightTitle->setAlignment(Qt::AlignCenter);
    mapsLayout->addWidget(heightTitle);
    
    heightMapLabel = new QLabel();
    heightMapLabel->setAlignment(Qt::AlignCenter);
    heightMapLabel->setMinimumSize(250, 250);
    heightMapLabel->setMaximumSize(300, 300);
    heightMapLabel->setStyleSheet("QLabel { background-color: #2b2b2b; border: 1px solid #555; }");
    mapsLayout->addWidget(heightMapLabel);
    
    // Biome map section
    QLabel *biomeTitle = new QLabel("Biome Map");
    biomeTitle->setFont(dimFont);
    biomeTitle->setAlignment(Qt::AlignCenter);
    mapsLayout->addWidget(biomeTitle);
    
    biomeMapLabel = new QLabel();
    biomeMapLabel->setAlignment(Qt::AlignCenter);
    biomeMapLabel->setMinimumSize(250, 250);
    biomeMapLabel->setMaximumSize(300, 300);
    biomeMapLabel->setStyleSheet("QLabel { background-color: #2b2b2b; border: 1px solid #555; }");
    mapsLayout->addWidget(biomeMapLabel);
    
    // Legend
    setupLegend();
    mapsLayout->addWidget(legendWidget);
    
    mapsLayout->addStretch();
    
    mainLayout->addLayout(controlsLayout);
    mainLayout->addLayout(mapsLayout, 1);
    mainLayout->setStretchFactor(controlsLayout, 0);
    mainLayout->setStretchFactor(mapsLayout, 1);
}

void MapBuilderWidget::connectControls()
{
    connect(seedBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MapBuilderWidget::onSeedChanged);
    connect(randomSeedButton, &QPushButton::clicked, this, &MapBuilderWidget::onRandomSeedClicked);
    connect(frequencyBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MapBuilderWidget::onFrequencyChanged);
    connect(octavesBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MapBuilderWidget::onOctavesChanged);
    connect(amplitudeBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MapBuilderWidget::onAmplitudeChanged);
    connect(persistenceBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MapBuilderWidget::onPersistenceChanged);
    connect(biomeFrequencyBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MapBuilderWidget::onBiomeFrequencyChanged);
    connect(biomeOctavesBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MapBuilderWidget::onBiomeOctavesChanged);
    connect(biomeWarpBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MapBuilderWidget::onBiomeWarpChanged);
    connect(mapWidthBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MapBuilderWidget::onMapWidthChanged);
    connect(mapHeightBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MapBuilderWidget::onMapHeightChanged);
    connect(saveMapSceneButton, &QPushButton::clicked, this, &MapBuilderWidget::onSaveMapScene);
}

void MapBuilderWidget::setupLegend()
{
    legendWidget = new QWidget();
    QVBoxLayout *legendLayout = new QVBoxLayout(legendWidget);
    legendLayout->setAlignment(Qt::AlignTop);
    
    QFont legendFont;
    legendFont.setPointSize(11);
    legendFont.setBold(true);
    QLabel *legendTitle = new QLabel("Biome Legend");
    legendTitle->setFont(legendFont);
    legendTitle->setAlignment(Qt::AlignCenter);
    legendLayout->addWidget(legendTitle);
    
    // Create legend entries
    QFont entryFont;
    entryFont.setPointSize(10);
    
    // Field (Yellow)
    QHBoxLayout *fieldLayout = new QHBoxLayout();
    QLabel *fieldColor = new QLabel();
    fieldColor->setFixedSize(30, 30);
    fieldColor->setStyleSheet("QLabel { background-color: rgb(255, 255, 0); border: 1px solid #555; }");
    QLabel *fieldLabel = new QLabel("Field");
    fieldLabel->setFont(entryFont);
    fieldLayout->addWidget(fieldColor);
    fieldLayout->addWidget(fieldLabel);
    fieldLayout->addStretch();
    legendLayout->addLayout(fieldLayout);
    
    // Mountains (Blue)
    QHBoxLayout *mountainsLayout = new QHBoxLayout();
    QLabel *mountainsColor = new QLabel();
    mountainsColor->setFixedSize(30, 30);
    mountainsColor->setStyleSheet("QLabel { background-color: rgb(0, 0, 255); border: 1px solid #555; }");
    QLabel *mountainsLabel = new QLabel("Mountains");
    mountainsLabel->setFont(entryFont);
    mountainsLayout->addWidget(mountainsColor);
    mountainsLayout->addWidget(mountainsLabel);
    mountainsLayout->addStretch();
    legendLayout->addLayout(mountainsLayout);
    
    // Forest (Green)
    QHBoxLayout *forestLayout = new QHBoxLayout();
    QLabel *forestColor = new QLabel();
    forestColor->setFixedSize(30, 30);
    forestColor->setStyleSheet("QLabel { background-color: rgb(0, 255, 0); border: 1px solid #555; }");
    QLabel *forestLabel = new QLabel("Forest");
    forestLabel->setFont(entryFont);
    forestLayout->addWidget(forestColor);
    forestLayout->addWidget(forestLabel);
    forestLayout->addStretch();
    legendLayout->addLayout(forestLayout);
    
    legendWidget->setStyleSheet("QWidget { background-color: #1e1e1e; border: 1px solid #555; padding: 10px; }");
}

void MapBuilderWidget::updateMapBuilderParams()
{
    m_mapBuilder.setSeed(seedBox->value());
    m_mapBuilder.setFrequency(static_cast<float>(frequencyBox->value()));
    m_mapBuilder.setOctaves(octavesBox->value());
    m_mapBuilder.setAmplitude(static_cast<float>(amplitudeBox->value()));
    m_mapBuilder.setPersistence(static_cast<float>(persistenceBox->value()));
    m_mapBuilder.setBiomeFrequency(static_cast<float>(biomeFrequencyBox->value()));
    m_mapBuilder.setBiomeOctaves(biomeOctavesBox->value());
    m_mapBuilder.setBiomeWarp(static_cast<float>(biomeWarpBox->value()));
    m_mapBuilder.setMapSize(mapWidthBox->value(), mapHeightBox->value());
}

void MapBuilderWidget::updateVisualizations()
{
    const auto& biomes = m_mapBuilder.getBiomes();
    const auto& heights = m_mapBuilder.getNormalizedHeights();
    
    int mapWidth = mapWidthBox->value();
    int mapHeight = mapHeightBox->value();
    
    if (mapWidth <= 0 || mapHeight <= 0 || biomes.empty()) {
        return;
    }
    
    // Generate biome map image
    delete biomeMapImage;
    biomeMapImage = new QImage(mapWidth, mapHeight, QImage::Format_RGB32);
    
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            BiomeType biome = biomes[y * mapWidth + x];
            unsigned char r, g, b;
            MapProperties::getBiomeColor(biome, r, g, b);
            biomeMapImage->setPixel(x, y, qRgb(r, g, b));
        }
    }
    
    // Display biome map
    QPixmap biomePixmap = QPixmap::fromImage(*biomeMapImage);
    if (biomeMapLabel && biomeMapLabel->size().width() > 0 && biomeMapLabel->size().height() > 0) {
        biomePixmap = biomePixmap.scaled(biomeMapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if (biomeMapLabel) {
        biomeMapLabel->setPixmap(biomePixmap);
    }
    
    // Generate height map image (only if heights are available)
    if (!heights.empty()) {
        delete heightMapImage;
        heightMapImage = new QImage(mapWidth, mapHeight, QImage::Format_Grayscale8);
        
        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                float normalized = heights[y * mapWidth + x];
                unsigned char gray = static_cast<unsigned char>(normalized * 255.0f);
                heightMapImage->setPixel(x, y, qRgb(gray, gray, gray));
            }
        }
        
        // Display height map
        QPixmap heightPixmap = QPixmap::fromImage(*heightMapImage);
        if (heightMapLabel && heightMapLabel->size().width() > 0 && heightMapLabel->size().height() > 0) {
            heightPixmap = heightPixmap.scaled(heightMapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        if (heightMapLabel) {
            heightMapLabel->setPixmap(heightPixmap);
        }
    }
}

void MapBuilderWidget::generateHeightMap()
{
    updateMapBuilderParams();
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
    updateMap(); // Update the 3D Map whenever height map is regenerated
}

void MapBuilderWidget::generateBiomeMap()
{
    updateMapBuilderParams();
    m_mapBuilder.generateBiomeMap();
    // Regenerate height map since biome changes affect height
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
    updateMap(); // Update the 3D Map whenever biome map is regenerated
}

void MapBuilderWidget::onSeedChanged(int value)
{
    generateBiomeMap();
    // Height map will be regenerated in generateBiomeMap()
}

void MapBuilderWidget::onRandomSeedClicked()
{
    // Generate random seed between 0 and 999999
    int randomSeed = std::rand() % 1000000;
    seedBox->setValue(randomSeed);
    // The valueChanged signal will trigger onSeedChanged which will regenerate the map
}

void MapBuilderWidget::onFrequencyChanged(double value)
{
    updateMapBuilderParams();
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
}

void MapBuilderWidget::onOctavesChanged(int value)
{
    updateMapBuilderParams();
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
}

void MapBuilderWidget::onAmplitudeChanged(double value)
{
    updateMapBuilderParams();
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
}

void MapBuilderWidget::onPersistenceChanged(double value)
{
    updateMapBuilderParams();
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
}

void MapBuilderWidget::onBiomeFrequencyChanged(double value)
{
    generateBiomeMap();
    // Regenerate height map since biome changes affect height
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
}

void MapBuilderWidget::onBiomeOctavesChanged(int value)
{
    generateBiomeMap();
    // Regenerate height map since biome changes affect height
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
}

void MapBuilderWidget::onBiomeWarpChanged(double value)
{
    generateBiomeMap();
    m_mapBuilder.generateHeightMap();
    updateVisualizations();
}

void MapBuilderWidget::onMapWidthChanged(int value)
{
    generateBiomeMap();
}

void MapBuilderWidget::onMapHeightChanged(int value)
{
    generateBiomeMap();
}

void MapBuilderWidget::onSaveMapScene()
{
    saveMapScene();
}


//This save was for testing and AI generated ChatGPT 5.1 2025
void MapBuilderWidget::saveMapScene()
{
    updateMapBuilderParams();
    m_mapBuilder.generateBiomeMap();
    m_mapBuilder.generateHeightMap();
    
    const auto& heights = m_mapBuilder.getNormalizedHeights();
    if (heights.empty()) {
        std::cout << "No height data available. Cannot save scene." << std::endl;
        return;
    }
    
    int mapWidth = mapWidthBox->value();
    int mapHeight = mapHeightBox->value();
    
    if (mapWidth <= 0 || mapHeight <= 0) {
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Map Scene"),
                                                    QDir::currentPath().append(QDir::separator())
                                                        .append("scenefiles")
                                                        .append(QDir::separator())
                                                        .append("realtime")
                                                        .append(QDir::separator())
                                                        .append("required")
                                                        .append(QDir::separator())
                                                        .append("map_scene.json"),
                                                    tr("Scene Files (*.json)"));
    
    if (filePath.isEmpty()) {
        return;
    }
    

    float mapCenterX = mapWidth * 0.5f;
    float mapCenterZ = mapHeight * 0.5f;
    float maxDimension = std::max(mapWidth, mapHeight);
    
    int middleX = mapWidth / 2;
    int middleZ = mapHeight / 2;
    const auto& normalizedHeights = m_mapBuilder.getNormalizedHeights();
    float middleHeight = normalizedHeights[middleZ * mapWidth + middleX];
    int middleBlockY = static_cast<int>(std::round(middleHeight * maxDimension * 0.1f)); // Use reduced scale
    
    int middleWorldX = middleX - static_cast<int>(mapCenterX);
    int middleWorldZ = middleZ - static_cast<int>(mapCenterZ);
    
    QJsonObject root;
    root["name"] = "root";
    
    // Global data
    QJsonObject globalData;
    globalData["ambientCoeff"] = 0.5;
    globalData["diffuseCoeff"] = 0.5;
    globalData["specularCoeff"] = 0.5;
    globalData["transparentCoeff"] = 0.0;
    root["globalData"] = globalData;
    
    // Camera data (position above and slightly offset from middle block for better view)
    QJsonObject cameraData;
    QJsonArray cameraPos;
    // Position camera above middle block, offset diagonally for better viewing angle
    int cameraOffset = std::max(5.0f, maxDimension / 4.0f); // Offset based on map size
    cameraPos.append(middleWorldX + cameraOffset);
    cameraPos.append(middleBlockY + std::max(10.0f, maxDimension / 3.0f)); // Higher for larger maps
    cameraPos.append(middleWorldZ + cameraOffset);
    cameraData["position"] = cameraPos;
    
    QJsonArray cameraUp;
    cameraUp.append(0.0);
    cameraUp.append(1.0);
    cameraUp.append(0.0);
    cameraData["up"] = cameraUp;
    
    QJsonArray cameraFocus;
    cameraFocus.append(middleWorldX);
    cameraFocus.append(middleBlockY);
    cameraFocus.append(middleWorldZ);
    cameraData["focus"] = cameraFocus;
    
    cameraData["heightAngle"] = 30.0;
    root["cameraData"] = cameraData;
    
    // Groups structure
    QJsonArray groups;
    
    // Light group
    QJsonObject lightGroup;
    QJsonArray lights;
    QJsonObject directionalLight;
    directionalLight["type"] = "directional";
    QJsonArray lightColor;
    lightColor.append(1.0);
    lightColor.append(1.0);
    lightColor.append(1.0);
    directionalLight["color"] = lightColor;
    QJsonArray lightDir;
    lightDir.append(0.0);
    lightDir.append(-1.0);
    lightDir.append(0.0);
    directionalLight["direction"] = lightDir;
    lights.append(directionalLight);
    lightGroup["lights"] = lights;
    groups.append(lightGroup);
    
    // Primitives group
    QJsonObject primitivesGroup;
    QJsonArray innerGroups;
    
    // Generate cubes for each map position
    const auto& biomes = m_mapBuilder.getBiomes();
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            float height = normalizedHeights[y * mapWidth + x];
            
            // Calculate world position (center map at origin)
            // Use integer coordinates like Minecraft
            int worldX = x - static_cast<int>(mapCenterX);
            int worldZ = y - static_cast<int>(mapCenterZ);
            // Height is already discrete integer, convert to integer world Y
            // Use reduced scale (0.1 instead of 0.5) to make terrain less hilly
            int worldY = static_cast<int>(std::round(height * maxDimension * 0.1f));
            
            // Create cube primitive with translation
            QJsonObject cubeGroup;
            QJsonArray translate;
            translate.append(worldX);
            translate.append(worldY);
            translate.append(worldZ);
            cubeGroup["translate"] = translate;
            
            QJsonArray primitives;
            QJsonObject cube;
            cube["type"] = "cube";
            
            // Color based on biome - get biome color and convert to 0-1 range
            BiomeType biome = biomes[y * mapWidth + x];
            unsigned char r, g, b;
            MapProperties::getBiomeColor(biome, r, g, b);
            QJsonArray diffuse;
            diffuse.append(r / 255.0);
            diffuse.append(g / 255.0);
            diffuse.append(b / 255.0);
            cube["diffuse"] = diffuse;
            
            QJsonArray specular;
            specular.append(1.0);
            specular.append(1.0);
            specular.append(1.0);
            cube["specular"] = specular;
            cube["shininess"] = 25.0;
            
            primitives.append(cube);
            cubeGroup["primitives"] = primitives;
            
            innerGroups.append(cubeGroup);
        }
    }
    
    primitivesGroup["groups"] = innerGroups;
    groups.append(primitivesGroup);
    
    root["groups"] = groups;
    
    // Write JSON to file
    QJsonDocument doc(root);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        std::cout << "Saved map scene to: \"" << filePath.toStdString() << "\"." << std::endl;
    } else {
        std::cout << "Failed to save map scene file." << std::endl;
    }
}

void MapBuilderWidget::updateMap()
{
    // For visualization: initialize from builder (limited size)
    m_map.initializeFromBuilder(m_mapBuilder);
    
    // Also set noise parameters for endless generation mode
    // This allows the actual game to use endless generation while
    // the world builder shows a preview
    m_map.setNoiseParams(m_mapBuilder.getParams());
}

void MapBuilderWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // Update image displays when widget is resized
    if (heightMapImage && !heightMapImage->isNull() && heightMapLabel) {
        QPixmap pixmap = QPixmap::fromImage(*heightMapImage);
        if (heightMapLabel->size().width() > 0 && heightMapLabel->size().height() > 0) {
            pixmap = pixmap.scaled(heightMapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        heightMapLabel->setPixmap(pixmap);
    }
    
    if (biomeMapImage && !biomeMapImage->isNull() && biomeMapLabel) {
        QPixmap pixmap = QPixmap::fromImage(*biomeMapImage);
        if (biomeMapLabel->size().width() > 0 && biomeMapLabel->size().height() > 0) {
            pixmap = pixmap.scaled(biomeMapLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        biomeMapLabel->setPixmap(pixmap);
    }
}
