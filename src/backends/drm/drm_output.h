/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KWIN_DRM_OUTPUT_H
#define KWIN_DRM_OUTPUT_H

#include "drm_abstract_output.h"
#include "drm_object.h"
#include "drm_object_plane.h"
#include "drm_pointer.h"

#include <QObject>
#include <QPoint>
#include <QSharedPointer>
#include <QSize>
#include <QTimer>
#include <QVector>
#include <chrono>
#include <xf86drmMode.h>

namespace KWin
{

class DrmBackend;
class DrmBuffer;
class DrmDumbBuffer;
class DrmPlane;
class DrmConnector;
class DrmCrtc;
class Cursor;
class DrmGpu;
class DrmPipeline;
class DumbSwapchain;

class KWIN_EXPORT DrmOutput : public DrmAbstractOutput
{
    Q_OBJECT
public:
    DrmOutput(DrmPipeline *pipeline);
    ~DrmOutput() override;

    DrmConnector *connector() const;
    DrmPipeline *pipeline() const;

    bool present() override;
    DrmOutputLayer *outputLayer() const override;

    bool queueChanges(const OutputConfiguration &config);
    void applyQueuedChanges(const OutputConfiguration &config);
    void revertQueuedChanges();
    void updateModes();

    bool usesSoftwareCursor() const override;

private:
    void initOutputDevice();

    void updateEnablement(bool enable) override;
    bool setDrmDpmsMode(DpmsMode mode);
    void setDpmsMode(DpmsMode mode) override;

    QVector<AbstractOutput::Mode> getModes() const;

    int gammaRampSize() const override;
    bool setGammaRamp(const GammaRamp &gamma) override;
    void updateCursor();
    void moveCursor();

    DrmPipeline *m_pipeline;
    DrmConnector *m_connector;

    QSharedPointer<DumbSwapchain> m_cursor;
    bool m_setCursorSuccessful = false;
    bool m_moveCursorSuccessful = false;
    QRect m_lastCursorGeometry;
    QTimer m_turnOffTimer;
};

}

Q_DECLARE_METATYPE(KWin::DrmOutput *)

#endif
