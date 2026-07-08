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
#include <QCheckBox>
#include <QFontMetrics>
#include <QGridLayout>
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
    wf_band_cb->setObjectName("wf_band_cb");
    sp_band_cb = new QComboBox(ui->Periodogram);
    sp_band_cb->setObjectName("sp_band_cb");

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
    wf_af_gain_sl->setObjectName("wf_af_gain_sl");
    sp_af_gain_sl = makeAFSlider(ui->Periodogram,  ui->AFGain_slide->value());
    sp_af_gain_sl->setObjectName("sp_af_gain_sl");
    wf_rf_gain_sl = makeRFSlider(ui->Waterfall,   ui->RFGain_slide->value());
    wf_rf_gain_sl->setObjectName("wf_rf_gain_sl");
    sp_rf_gain_sl = makeRFSlider(ui->Periodogram,  ui->RFGain_slide->value());
    sp_rf_gain_sl->setObjectName("sp_rf_gain_sl");

    // Inject into control columns —————————————————————————————————————————————

    // Mode + AF Filter mirror comboboxes.  Share the master's model so items
    // added later (via radio_listener signals wired in main_setup_top.cpp)
    // show up in the mirrors automatically.
    QComboBox * wf_mode_cb = new QComboBox(ui->Waterfall);
    wf_mode_cb->setObjectName("wf_mode_cb");
    wf_mode_cb->setModel(ui->Mode_cb->model());
    wf_mode_cb->setCurrentIndex(ui->Mode_cb->currentIndex());
    QComboBox * sp_mode_cb = new QComboBox(ui->Periodogram);
    sp_mode_cb->setObjectName("sp_mode_cb");
    sp_mode_cb->setModel(ui->Mode_cb->model());
    sp_mode_cb->setCurrentIndex(ui->Mode_cb->currentIndex());
    QComboBox * wf_afbw_cb = new QComboBox(ui->Waterfall);
    wf_afbw_cb->setObjectName("wf_afbw_cb");
    wf_afbw_cb->setModel(ui->AFBw_cb->model());
    wf_afbw_cb->setCurrentIndex(ui->AFBw_cb->currentIndex());
    QComboBox * sp_afbw_cb = new QComboBox(ui->Periodogram);
    sp_afbw_cb->setObjectName("sp_afbw_cb");
    sp_afbw_cb->setModel(ui->AFBw_cb->model());
    sp_afbw_cb->setCurrentIndex(ui->AFBw_cb->currentIndex());

    struct PanelEntry {
        QWidget   * tab;
        QComboBox * band;
        QComboBox * mode;
        QComboBox * afbw;
    };
    for (const PanelEntry & e : {
            PanelEntry{ui->Waterfall,   wf_band_cb, wf_mode_cb, wf_afbw_cb},
            PanelEntry{ui->Periodogram, sp_band_cb, sp_mode_cb, sp_afbw_cb}}) {
        QBoxLayout * col = spectCtrlCol(e.tab);
        if (!col) continue;
        // Insert before the trailing stretch spacer.  A leading stretch pushes
        // Band/Mode/AF Filter away from the Span combobox above.
        int ins = col->count() - 1;
        col->insertStretch(ins++, 1);
        col->insertWidget(ins++, new QLabel(tr("Band"), e.tab));
        col->insertWidget(ins++, e.band);
        col->insertWidget(ins++, new QLabel(tr("Mode"), e.tab));
        col->insertWidget(ins++, e.mode);
        col->insertWidget(ins++, new QLabel(tr("AF Filter"), e.tab));
        col->insertWidget(ins++, e.afbw);
    }

    // Bottom strip on each panel: AF Gain slider, RF Gain slider, Record AF,
    // Record RF.  Placed in a QGridLayout that shares columns with the plot
    // area so the strip's left edge lines up with the plot's left edge.
    struct StripEntry {
        QWidget    * tab;
        QSlider    * af;
        QSlider    * rf;
        QCheckBox ** rec_af_out;
        QCheckBox ** rec_rf_out;
    };
    QCheckBox * wf_rec_af = nullptr, * wf_rec_rf = nullptr;
    QCheckBox * sp_rec_af = nullptr, * sp_rec_rf = nullptr;
    for (const StripEntry & e : {
            StripEntry{ui->Waterfall,   wf_af_gain_sl, wf_rf_gain_sl, &wf_rec_af, &wf_rec_rf},
            StripEntry{ui->Periodogram, sp_af_gain_sl, sp_rf_gain_sl, &sp_rec_af, &sp_rec_rf}}) {
        auto * tabLay = qobject_cast<QVBoxLayout*>(e.tab->layout());
        if (!tabLay) continue;

        // Extract [leftCol | plot] from the existing HBox so we can place them
        // in a grid that has a second row for the strip.
        QLayoutItem * hlayItem = tabLay->takeAt(0);
        if (!hlayItem) continue;
        auto * hlay = qobject_cast<QHBoxLayout*>(hlayItem->layout());
        if (!hlay) { tabLay->insertItem(0, hlayItem); continue; }
        QLayoutItem * leftItem = hlay->takeAt(0);
        QLayoutItem * plotItem = hlay->takeAt(0);
        if (!leftItem || !plotItem) {
            if (leftItem) hlay->addItem(leftItem);
            if (plotItem) hlay->addItem(plotItem);
            tabLay->insertItem(0, hlayItem);
            continue;
        }
        delete hlayItem;   // deletes the now-empty hlay

        auto * rec_af = new QCheckBox(tr("Record AF"), e.tab);
        rec_af->setChecked(ui->Record_chk->isChecked());
        auto * rec_rf = new QCheckBox(tr("Record RF"), e.tab);
        rec_rf->setChecked(ui->RecordRF_chk->isChecked());
        *e.rec_af_out = rec_af;
        *e.rec_rf_out = rec_rf;

        // Slider width: 1.5× the width of the "RF Gain (dB)" label text.
        QFontMetrics fm(e.tab->font());
        int slW = (fm.horizontalAdvance(tr("RF Gain (dB)")) * 3) / 2;
        e.af->setMaximumWidth(slW);
        e.rf->setMaximumWidth(slW);

        auto * strip = new QHBoxLayout();
        strip->setContentsMargins(4, 2, 4, 2);
        strip->addWidget(new QLabel(tr("AF Gain"), e.tab));
        strip->addWidget(e.af);
        strip->addWidget(new QLabel(tr("RF Gain (dB)"), e.tab));
        strip->addWidget(e.rf);
        strip->addStretch(1);
        strip->addWidget(rec_af);
        strip->addWidget(rec_rf);

        auto * grid = new QGridLayout();
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setHorizontalSpacing(2);
        grid->setVerticalSpacing(2);
        grid->addItem(leftItem, 0, 0);
        grid->addItem(plotItem, 0, 1);
        grid->addLayout(strip,  1, 1);
        grid->setColumnStretch(0, 1);
        grid->setColumnStretch(1, 25);
        grid->setRowStretch(0, 1);
        grid->setRowStretch(1, 0);
        tabLay->addLayout(grid);
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

    // Record AF / Record RF connections —————————————————————————————————————
    // Mirror → master: setChecked on master fires its existing signal that
    // drives the recorder AND the master→mirrors sync below.
    connect(wf_rec_af, &QCheckBox::toggled, this,
            [this](bool v) { ui->Record_chk->setChecked(v); });
    connect(sp_rec_af, &QCheckBox::toggled, this,
            [this](bool v) { ui->Record_chk->setChecked(v); });
    connect(wf_rec_rf, &QCheckBox::toggled, this,
            [this](bool v) { ui->RecordRF_chk->setChecked(v); });
    connect(sp_rec_rf, &QCheckBox::toggled, this,
            [this](bool v) { ui->RecordRF_chk->setChecked(v); });

    // Master → mirrors.  Blockers keep the mirror→master path from re-firing.
    connect(ui->Record_chk, &QCheckBox::toggled, this,
            [wf_rec_af, sp_rec_af](bool v) {
        if (wf_rec_af) { QSignalBlocker bwf(wf_rec_af); wf_rec_af->setChecked(v); }
        if (sp_rec_af) { QSignalBlocker bsp(sp_rec_af); sp_rec_af->setChecked(v); }
    });
    connect(ui->RecordRF_chk, &QCheckBox::toggled, this,
            [wf_rec_rf, sp_rec_rf](bool v) {
        if (wf_rec_rf) { QSignalBlocker bwf(wf_rec_rf); wf_rec_rf->setChecked(v); }
        if (sp_rec_rf) { QSignalBlocker bsp(sp_rec_rf); sp_rec_rf->setChecked(v); }
    });

    // Mode / AF Filter connections —————————————————————————————————————————
    // Mirror → master: setCurrentText fires master's normal signals, so the
    // radio-side slot (connected in main_setup_top.cpp) runs.
    connect(wf_mode_cb, &QComboBox::currentTextChanged, this,
            [this](const QString & t) { ui->Mode_cb->setCurrentText(t); });
    connect(sp_mode_cb, &QComboBox::currentTextChanged, this,
            [this](const QString & t) { ui->Mode_cb->setCurrentText(t); });
    connect(wf_afbw_cb, &QComboBox::currentTextChanged, this,
            [this](const QString & t) { ui->AFBw_cb->setCurrentText(t); });
    connect(sp_afbw_cb, &QComboBox::currentTextChanged, this,
            [this](const QString & t) { ui->AFBw_cb->setCurrentText(t); });

    // Master → mirrors (blocked to break the loop).
    connect(ui->Mode_cb, &QComboBox::currentTextChanged, this,
            [wf_mode_cb, sp_mode_cb](const QString & t) {
        if (wf_mode_cb) { QSignalBlocker bwf(wf_mode_cb); wf_mode_cb->setCurrentText(t); }
        if (sp_mode_cb) { QSignalBlocker bsp(sp_mode_cb); sp_mode_cb->setCurrentText(t); }
    });
    connect(ui->AFBw_cb, &QComboBox::currentTextChanged, this,
            [wf_afbw_cb, sp_afbw_cb](const QString & t) {
        if (wf_afbw_cb) { QSignalBlocker bwf(wf_afbw_cb); wf_afbw_cb->setCurrentText(t); }
        if (sp_afbw_cb) { QSignalBlocker bsp(sp_afbw_cb); sp_afbw_cb->setCurrentText(t); }
    });
}
