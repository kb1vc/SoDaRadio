/*
Copyright (c) 2025 Matthew H. Reilly (kb1vc)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// ShortSoDa-specific: injects band selector, AF gain slider, and RF gain
// slider into the control columns of the Waterfall and Periodogram panels.
// Not compiled for the full qtgui build.

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QBoxLayout>
#include <QSignalBlocker>
#include "RadioListener.hpp"

// Returns the narrow control VBox on the left side of a wf/spect tab.
// Layout path: tab->layout()[VBox] -> [0].layout()[HBox] -> [0].layout()[VBox]
static QBoxLayout * spectCtrlCol(QWidget * tab)
{
    auto * tabVBox = qobject_cast<QBoxLayout*>(tab->layout());
    if (!tabVBox) return nullptr;
    QLayoutItem * item0 = tabVBox->itemAt(0);
    if (!item0) return nullptr;
    auto * hlay = qobject_cast<QBoxLayout*>(item0->layout());
    if (!hlay) return nullptr;
    QLayoutItem * col0 = hlay->itemAt(0);
    if (!col0) return nullptr;
    return qobject_cast<QBoxLayout*>(col0->layout());
}

void MainWindow::setupSpectControls()
{
    // Create widgets —————————————————————————————————————————————————————————

    wf_band_cb = new QComboBox(ui->Waterfall);
    sp_band_cb = new QComboBox(ui->Periodogram);

    // AF gain: 0–100 matching AFGain_slide
    auto makeAFSlider = [](QWidget * parent, int cur) {
        auto * s = new QSlider(Qt::Horizontal, parent);
        s->setRange(0, 100);
        s->setTickInterval(10);
        s->setTickPosition(QSlider::TicksBothSides);
        s->setValue(cur);
        return s;
    };

    // RF gain: -40–0 matching RFGain_slide
    auto makeRFSlider = [](QWidget * parent, int cur) {
        auto * s = new QSlider(Qt::Horizontal, parent);
        s->setRange(-40, 0);
        s->setTickInterval(10);
        s->setTickPosition(QSlider::TicksBothSides);
        s->setValue(cur);
        return s;
    };

    wf_af_gain_sl = makeAFSlider(ui->Waterfall,   ui->AFGain_slide->value());
    sp_af_gain_sl = makeAFSlider(ui->Periodogram,  ui->AFGain_slide->value());
    wf_rf_gain_sl = makeRFSlider(ui->Waterfall,   ui->RFGain_slide->value());
    sp_rf_gain_sl = makeRFSlider(ui->Periodogram,  ui->RFGain_slide->value());

    // Inject into control columns —————————————————————————————————————————————

    struct PanelEntry {
        QWidget   * tab;
        QComboBox * band;
        QSlider   * af;
        QSlider   * rf;
    };
    for (const PanelEntry & e : {
            PanelEntry{ui->Waterfall,   wf_band_cb, wf_af_gain_sl, wf_rf_gain_sl},
            PanelEntry{ui->Periodogram, sp_band_cb, sp_af_gain_sl, sp_rf_gain_sl}}) {
        QBoxLayout * col = spectCtrlCol(e.tab);
        if (!col) continue;
        // Insert before the trailing stretch spacer.
        int ins = col->count() - 1;
        col->insertWidget(ins++, new QLabel(tr("Band"), e.tab));
        col->insertWidget(ins++, e.band);
        col->insertWidget(ins++, new QLabel(tr("AF Gain"), e.tab));
        col->insertWidget(ins++, e.af);
        col->insertWidget(ins++, new QLabel(tr("RF Gain (dB)"), e.tab));
        col->insertWidget(ins++, e.rf);
    }

    // Band selector connections ————————————————————————————————————————————————

    // Mirror → master: updating the master fires changeBand via the existing
    // connection in main_setup_bandconfig.cpp, so we block it and call directly
    // to avoid a double changeBand call.
    connect(wf_band_cb, &QComboBox::currentTextChanged, this,
            [this](const QString & band) {
        if (sp_band_cb) { QSignalBlocker bsp(sp_band_cb); sp_band_cb->setCurrentText(band); }
        { QSignalBlocker bm(ui->bandSel_cb); ui->bandSel_cb->setCurrentText(band); }
        changeBand(band);
    });
    connect(sp_band_cb, &QComboBox::currentTextChanged, this,
            [this](const QString & band) {
        if (wf_band_cb) { QSignalBlocker bwf(wf_band_cb); wf_band_cb->setCurrentText(band); }
        { QSignalBlocker bm(ui->bandSel_cb); ui->bandSel_cb->setCurrentText(band); }
        changeBand(band);
    });

    // Master → mirrors (changeBand already wired to bandSel_cb elsewhere)
    connect(ui->bandSel_cb, &QComboBox::currentTextChanged, this,
            [this](const QString & band) {
        if (wf_band_cb) { QSignalBlocker bwf(wf_band_cb); wf_band_cb->setCurrentText(band); }
        if (sp_band_cb) { QSignalBlocker bsp(sp_band_cb); sp_band_cb->setCurrentText(band); }
    });

    // AF gain connections ——————————————————————————————————————————————————————

    // Mirror → master (block master to avoid double radio call; call directly)
    connect(wf_af_gain_sl, &QSlider::valueChanged, this, [this](int v) {
        { QSignalBlocker bm(ui->AFGain_slide); ui->AFGain_slide->setValue(v); }
        if (sp_af_gain_sl) { QSignalBlocker bsp(sp_af_gain_sl); sp_af_gain_sl->setValue(v); }
        radio_listener->setAFGain(v);
    });
    connect(sp_af_gain_sl, &QSlider::valueChanged, this, [this](int v) {
        { QSignalBlocker bm(ui->AFGain_slide); ui->AFGain_slide->setValue(v); }
        if (wf_af_gain_sl) { QSignalBlocker bwf(wf_af_gain_sl); wf_af_gain_sl->setValue(v); }
        radio_listener->setAFGain(v);
    });

    // Master → mirrors (radio already notified by existing connection)
    connect(ui->AFGain_slide, &QSlider::valueChanged, this, [this](int v) {
        if (wf_af_gain_sl) { QSignalBlocker bwf(wf_af_gain_sl); wf_af_gain_sl->setValue(v); }
        if (sp_af_gain_sl) { QSignalBlocker bsp(sp_af_gain_sl); sp_af_gain_sl->setValue(v); }
    });

    // RF gain connections ——————————————————————————————————————————————————————

    connect(wf_rf_gain_sl, &QSlider::valueChanged, this, [this](int v) {
        { QSignalBlocker bm(ui->RFGain_slide); ui->RFGain_slide->setValue(v); }
        if (sp_rf_gain_sl) { QSignalBlocker bsp(sp_rf_gain_sl); sp_rf_gain_sl->setValue(v); }
        radio_listener->setRXGain(v);
    });
    connect(sp_rf_gain_sl, &QSlider::valueChanged, this, [this](int v) {
        { QSignalBlocker bm(ui->RFGain_slide); ui->RFGain_slide->setValue(v); }
        if (wf_rf_gain_sl) { QSignalBlocker bwf(wf_rf_gain_sl); wf_rf_gain_sl->setValue(v); }
        radio_listener->setRXGain(v);
    });

    // Master → mirrors (radio already notified by existing connection in
    // main_setup_top.cpp via connect(RFGain_slide, valueChanged, setRXGain))
    connect(ui->RFGain_slide, &QSlider::valueChanged, this, [this](int v) {
        if (wf_rf_gain_sl) { QSignalBlocker bwf(wf_rf_gain_sl); wf_rf_gain_sl->setValue(v); }
        if (sp_rf_gain_sl) { QSignalBlocker bsp(sp_rf_gain_sl); sp_rf_gain_sl->setValue(v); }
    });
}
