// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include "common/param_package.h"
#include "common/scm_rev.h"
#include "core/core.h"
#include "hid_core/frontend/emulated_controller.h"
#include "hid_core/hid_core.h"
#include "hid_core/hid_types.h"
#include "input_common/main.h"
#include "qt_common/config/uisettings.h"
#include "yuzu/configuration/configure_controllers.h"
#include "yuzu/configuration/configure_input.h"

namespace {
QString NpadStyleName(Core::HID::NpadStyleIndex type) {
    using Core::HID::NpadStyleIndex;
    switch (type) {
    case NpadStyleIndex::Fullkey:
        return QObject::tr("Pro Controller");
    case NpadStyleIndex::Handheld:
        return QObject::tr("Handheld");
    case NpadStyleIndex::JoyconDual:
        return QObject::tr("Dual Joy-Con");
    case NpadStyleIndex::JoyconLeft:
        return QObject::tr("Left Joy-Con");
    case NpadStyleIndex::JoyconRight:
        return QObject::tr("Right Joy-Con");
    case NpadStyleIndex::GameCube:
        return QObject::tr("GameCube Controller");
    default:
        return QObject::tr("Controller");
    }
}
} // namespace

ConfigureControllers::ConfigureControllers(Core::System& system_,
                                           InputCommon::InputSubsystem* input_subsystem_,
                                           QWidget* parent)
    : QWidget(parent), system{system_}, input_subsystem{input_subsystem_} {
    // Used by ConfigureDialog::UpdateVisibleTabs() as the inner tab label.
    setAccessibleName(tr("Controllers"));

    auto* const root = new QVBoxLayout(this);

    auto* const title = new QLabel(tr("Controllers"), this);
    QFont title_font = title->font();
    title_font.setPointSize(title_font.pointSize() + 4);
    title_font.setBold(true);
    title->setFont(title_font);
    root->addWidget(title);

    auto* const info = new QLabel(
        tr("Controllers are managed by Steam. Add or reorder controllers in Steam's controller "
           "settings - Eden assigns them to players automatically in that order, as Pro "
           "Controllers. No per-button setup is needed."),
        this);
    info->setWordWrap(true);
    root->addWidget(info);

    auto* const status_group = new QGroupBox(tr("Connected controllers"), this);
    auto* const status_layout = new QVBoxLayout(status_group);
    for (auto*& label : status_labels) {
        label = new QLabel(status_group);
        label->setVisible(false);
        status_layout->addWidget(label);
    }
    status_empty_label = new QLabel(tr("No controllers detected."), status_group);
    status_layout->addWidget(status_empty_label);
    root->addWidget(status_group);

    auto_map_checkbox = new QCheckBox(tr("Auto-map controllers using Steam's order"), this);
    auto_map_checkbox->setChecked(UISettings::values.auto_map_controllers.GetValue());
    auto_map_checkbox->setToolTip(
        tr("When on, controllers are bound to players automatically in the order Steam reports "
           "them, as Pro Controllers."));
    root->addWidget(auto_map_checkbox);
    // Persisted in ApplyConfiguration() on OK (so Cancel correctly reverts), like every other
    // setting; the 1s auto-map timer then picks up the new value.

    auto* const hint = new QLabel(
        tr("Recommended for Steam Deck. Turn off only if you want to set up player bindings "
           "manually under Advanced."),
        this);
    hint->setWordWrap(true);
    // Keep it readable (dimmed, not disabled-greyed) - this is exactly the guidance a first-time
    // Deck user needs at arm's length.
    hint->setStyleSheet(QStringLiteral("color: #8b8b8b;"));
    root->addWidget(hint);

    root->addStretch(1);

    advanced_button = new QPushButton(tr("Advanced - manual controller setup..."), this);
    root->addWidget(advanced_button);
    connect(advanced_button, &QPushButton::clicked, this, &ConfigureControllers::OpenAdvanced);

    auto* const version_label =
        new QLabel(tr("VibeEden version: %1").arg(QString::fromLatin1(Common::g_build_version)),
                   this);
    version_label->setStyleSheet(QStringLiteral("color: #8b8b8b;"));
    root->addWidget(version_label);

    // Poll live controller status (cheap; avoids cross-thread HID callbacks).
    refresh_timer = new QTimer(this);
    refresh_timer->setInterval(750);
    connect(refresh_timer, &QTimer::timeout, this, &ConfigureControllers::RefreshStatus);
    refresh_timer->start();
    RefreshStatus();
}

ConfigureControllers::~ConfigureControllers() = default;

void ConfigureControllers::ApplyConfiguration() {
    UISettings::values.auto_map_controllers.SetValue(auto_map_checkbox->isChecked());
}

void ConfigureControllers::RefreshStatus() {
    // Don't poll HID while this page isn't the visible tab.
    if (!isVisible()) {
        return;
    }

    // Label the source accurately: "Steam Input" only when a Steam-virtual pad is actually present,
    // otherwise the auto-mapper fell back to a plain SDL gamepad.
    bool any_steam_virtual = false;
    for (const auto& device : input_subsystem->GetInputDevices()) {
        if (device.Get("engine", "") == "sdl" && device.Get("steam_virtual", 0) != 0) {
            any_steam_virtual = true;
            break;
        }
    }
    const QString source = any_steam_virtual ? tr("Steam Input") : tr("Gamepad");

    auto& hid_core = system.HIDCore();
    int connected = 0;
    for (std::size_t i = 0; i < status_labels.size(); ++i) {
        const auto* const controller = hid_core.GetEmulatedControllerByIndex(i);
        const bool is_connected = controller != nullptr && controller->IsConnected();
        if (is_connected) {
            status_labels[i]->setText(tr("Player %1: %2 - %3")
                                          .arg(static_cast<int>(i) + 1)
                                          .arg(NpadStyleName(controller->GetNpadStyleIndex()))
                                          .arg(source));
            status_labels[i]->setVisible(true);
            ++connected;
        } else {
            status_labels[i]->setVisible(false);
        }
    }
    status_empty_label->setVisible(connected == 0);
}

void ConfigureControllers::OpenAdvanced() {
    emit AdvancedEditorVisibilityChanged(true);

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Advanced controller setup"));
    dialog.setWindowModality(Qt::WindowModal);

    auto* const layout = new QVBoxLayout(&dialog);

    // Reuse the full legacy per-player editor unchanged. It must be shown self-contained (do NOT
    // call GetSubTabs(), which would re-parent its tabs out of its own QTabWidget).
    auto* const input = new ConfigureInput(system, &dialog);
    input->Initialize(input_subsystem);
    layout->addWidget(input);

    auto* const buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        input->ApplyConfiguration();
    }

    emit AdvancedEditorVisibilityChanged(false);
    RefreshStatus();
}
