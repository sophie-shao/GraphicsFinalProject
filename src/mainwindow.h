#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QPushButton>
#include <QColorDialog>
#include "realtime.h"
#include "map/mapbuilderwidget.h" 
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

    Realtime *realtime;
    AspectRatioWidget *aspectRatioWidget;

    QPushButton *uploadFile;
    QPushButton *saveImage;
    QSlider *p1Slider;
    QSlider *p2Slider;
    QSpinBox *p1Box;
    QSpinBox *p2Box;
    QSlider *nearSlider;
    QSlider *farSlider;
    QDoubleSpinBox *nearBox;
    QDoubleSpinBox *farBox;

    // Telemetry labels
    QLabel *cameraPosLabel;
    QLabel *chunkPosLabel;

    // Camera controls
    QCheckBox *flyingModeCheckbox;
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

    // Extra Credit:
    QCheckBox *ec1;
    QCheckBox *ec2;
    QCheckBox *ec3;
    QCheckBox *ec4;

private slots:
    // From old Project 6
    // void onPerPixelFilter();
    // void onKernelBasedFilter();

    void onUploadFile();
    void onSaveImage();
    void onValChangeP1(int newValue);
    void onValChangeP2(int newValue);
    void onValChangeNearSlider(int newValue);
    void onValChangeFarSlider(int newValue);
    void onValChangeNearBox(double newValue);
    void onValChangeFarBox(double newValue);

    // Telemetry updates
    void onTelemetryUpdate(float x, float y, float z, int chunkX, int chunkZ);

    // Camera controls
    void onFlyingModeChanged();
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

    // Extra Credit:
    void onExtraCredit1();
    void onExtraCredit2();
    void onExtraCredit3();
    void onExtraCredit4();
};
