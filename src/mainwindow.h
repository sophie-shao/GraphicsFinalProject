#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include "realtime.h"
#include "map/mapbuilderwidget.h"
#include "lsystem/lsystemwidget.h"
#include "lsystem/treegenerator.h"
#include "utils/aspectratiowidget/aspectratiowidget.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void connectUIElements();
    void connectParam1();
    void connectParam2();
    void connectNear();
    void connectFar();

    // From old Project 6
    // void connectPerPixelFilter();
    // void connectKernelBasedFilter();

    

    void connectUploadFile();
    void connectSaveImage();
    void connectExtraCredit();

    //ADDED FOR FP
    QTabWidget *tabWidget;
    QWidget *renderTab;
    MapBuilderWidget *mapBuilderWidget;
    QWidget *lSystemTab;
    LSystemWidget *lSystemWidget;

    Realtime *realtime;
    AspectRatioWidget *aspectRatioWidget;

    QPushButton *uploadFile;
    QPushButton *saveImage;
    QSlider *nearSlider;
    QSlider *farSlider;
    QDoubleSpinBox *nearBox;
    QDoubleSpinBox *farBox;

    // Telemetry labels
    QLabel *cameraPosLabel;
    QLabel *chunkPosLabel;

    // Camera controls
    QCheckBox *flyingModeCheckbox;
    QCheckBox *fpsModeCheckbox;
    QCheckBox *motionBlurCheckbox;
    QCheckBox *depthVisualizationCheckbox;
    QComboBox *gbufferVizComboBox;
    QSlider *motionBlurSamplesSlider;
    QSpinBox *motionBlurSamplesBox;
    QSlider *movementSpeedSlider;
    QDoubleSpinBox *movementSpeedBox;
    QSlider *jumpHeightSlider;
    QDoubleSpinBox *jumpHeightBox;
    QSlider *cameraHeightSlider;
    QDoubleSpinBox *cameraHeightBox;
    QPushButton *playerColorButton;
    QSlider *gravitySlider;
    QDoubleSpinBox *gravityBox;
    QSlider *overheadLightSlider;
    QDoubleSpinBox *overheadLightBox;
    
    QPushButton *addPathPointButton;
    QPushButton *playPathButton;
    QPushButton *stopPathButton;
    QPushButton *clearPathButton;
    QSlider *pathDurationSlider;
    QDoubleSpinBox *pathDurationBox;
    
    QCheckBox *enemyAutoSpawnCheckbox;
    QSlider *enemySpawnDelaySlider;
    QDoubleSpinBox *enemySpawnDelayBox;
    QPushButton *enemyManualSpawnButton;
    QPushButton *teleportButton;
    
    QCheckBox *fogCheckbox;
    QCheckBox *flashlightCheckbox;
    QLabel *flashlightChargeLabel;
    QPushButton *fogColorButton;
    QSlider *fogIntensitySlider;
    QDoubleSpinBox *fogIntensityBox;
    
    //post-processing filters
    QGroupBox *filterGroupBox;
    QButtonGroup *filterButtonGroup;
    QCheckBox *filterNoneCheckbox;
    QCheckBox *filterWaveCheckbox;
    QCheckBox *filterShakeCheckbox;
    QCheckBox *filterInkOutlineCheckbox;
    QCheckBox *filterCartoonCheckbox;
    QCheckBox *filterStaticNoiseCheckbox;
    QCheckBox *filterDepthCheckbox;
    QCheckBox *filterColorGradingCheckbox;
    QCheckBox *filterPixelateCheckbox;
    QPushButton *lutSelectButton;
    QCheckBox *grainOverlayCheckbox;
    QSlider *grainOpacitySlider;
    QDoubleSpinBox *grainOpacityBox;
    
    //for particle system
    QGroupBox *particleGroupBox;
    QCheckBox *particlesEnabledCheckbox;
    QCheckBox *dirtParticlesCheckbox;
    QCheckBox *fogWispsCheckbox;
    QSlider *dirtSpawnRateSlider;
    QDoubleSpinBox *dirtSpawnRateBox;
    QSlider *fogWispIntervalSlider;
    QDoubleSpinBox *fogWispIntervalBox;
    QSlider *maxParticlesSlider;
    QSpinBox *maxParticlesBox;

    QCheckBox *ec1;
    QCheckBox *ec2;
    QCheckBox *ec3;
    QCheckBox *ec4;

private slots:

    void onUploadFile();
    void onSaveImage();
    void onValChangeNearSlider(int newValue);
    void onValChangeFarSlider(int newValue);
    void onValChangeNearBox(double newValue);
    void onValChangeFarBox(double newValue);

    //telemetry updates
    void onTelemetryUpdate(float x, float y, float z, int chunkX, int chunkZ);

    //camera controls
    void onFlyingModeChanged();
    void onFpsModeChanged();
    void onMotionBlurChanged();
    void onDepthVisualizationChanged();
    void onGBufferVizChanged(int index);
    void onMotionBlurSamplesChanged(int value);
    void onMotionBlurSamplesBoxChanged(int value);
    void onMovementSpeedChanged(int value);
    void onMovementSpeedBoxChanged(double value);
    void onJumpHeightChanged(int value);
    void onJumpHeightBoxChanged(double value);
    void onCameraHeightChanged(int value);
    void onCameraHeightBoxChanged(double value);
    void onPlayerColorChanged();
    void onGravityChanged(int value);
    void onGravityBoxChanged(double value);
    void onOverheadLightChanged(int value);
    void onOverheadLightBoxChanged(double value);
    void onAddPathPoint();
    void onPlayPath();
    void onStopPath();
    void onClearPath();
    void onPathDurationChanged(int value);
    void onPathDurationBoxChanged(double value);
    
    void onEnemyAutoSpawnChanged(int state);
    void onEnemySpawnDelayChanged(int value);
    void onEnemySpawnDelayBoxChanged(double value);
    void onEnemyManualSpawn();
    
    void onFogChanged();
    void onFlashlightChanged();
    void onFlashlightChargeChanged(float charge, bool inPenalty);
    void onTeleportToOrigin();
    void onFogColorChanged();
    void onFogIntensityChanged(int value);
    void onFogIntensityBoxChanged(double value);

    //post-processing filters
    void onFilterNoneChanged();
    void onFilterWaveChanged();
    void onFilterShakeChanged();
    void onFilterInkOutlineChanged();
    void onFilterCartoonChanged();
    void onFilterStaticNoiseChanged();
    void onFilterDepthChanged();
    void onFilterColorGradingChanged();
    void onFilterPixelateChanged();
    void onGrainOverlayChanged();
    void onGrainOpacityChanged();
    void onLUTSelect();
    
    //particle system
    void onParticlesEnabledChanged();
    void onDirtParticlesChanged();
    void onFogWispsChanged();
    void onDirtSpawnRateChanged(int value);
    void onDirtSpawnRateBoxChanged(double value);
    void onFogWispIntervalChanged(int value);
    void onFogWispIntervalBoxChanged(double value);
    void onMaxParticlesChanged(int value);
    void onMaxParticlesBoxChanged(int value);

    //extra Credit:
    void onExtraCredit1();
    void onExtraCredit2();
    void onExtraCredit3();
    void onExtraCredit4();
};
