#pragma once

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QImage>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <vector>
#include "mapproperties.h"
#include "mapbuilder.h"
#include "Map.h"

class MapBuilderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapBuilderWidget(QWidget *parent = nullptr);
    void generateHeightMap();
    void generateBiomeMap();
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void connectControls();
    void setupLegend();
    void updateMapBuilderParams();
    void updateVisualizations();
    void saveMapScene();
    void updateMap();
    
public:
    Map* getMap() { return &m_map; }
    
    QLabel *heightMapLabel;
    QLabel *biomeMapLabel;
    QImage *heightMapImage;
    QImage *biomeMapImage;
    
    QWidget *legendWidget;
    
    MapBuilder m_mapBuilder;
    
    Map m_map;
    
    QSpinBox *seedBox;
    QPushButton *randomSeedButton;
    QPushButton *saveMapSceneButton;
    QDoubleSpinBox *frequencyBox;
    QSpinBox *octavesBox;
    QDoubleSpinBox *amplitudeBox;
    QDoubleSpinBox *persistenceBox;
    QDoubleSpinBox *biomeFrequencyBox;
    QSpinBox *biomeOctavesBox;
    QDoubleSpinBox *biomeWarpBox;
    QSpinBox *mapWidthBox;
    QSpinBox *mapHeightBox;

private slots:
    void onSeedChanged(int value);
    void onRandomSeedClicked();
    void onFrequencyChanged(double value);
    void onOctavesChanged(int value);
    void onAmplitudeChanged(double value);
    void onPersistenceChanged(double value);
    void onBiomeFrequencyChanged(double value);
    void onBiomeOctavesChanged(int value);
    void onBiomeWarpChanged(double value);
    void onMapWidthChanged(int value);
    void onMapHeightChanged(int value);
    void onSaveMapScene();
};
