// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <QWidget>

namespace Core {
class System;
}

namespace InputCommon {
class InputSubsystem;
}

class QCheckBox;
class QLabel;
class QPushButton;
class QTimer;

/**
 * The default, Deck-first "Controllers" page of the settings dialog.
 *
 * Controllers are managed by Steam: the user adds/reorders them in Steam's controller settings and
 * Eden assigns them to players automatically (see SteamControllerMapper), so this page deliberately
 * has no per-button setup. It shows live per-player status, a single "Auto-map controllers" toggle,
 * and an "Advanced" button that opens the full legacy per-player editor (ConfigureInput) in a modal
 * for power users / unusual controllers.
 */
class ConfigureControllers : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureControllers(Core::System& system_,
                                  InputCommon::InputSubsystem* input_subsystem_,
                                  QWidget* parent = nullptr);
    ~ConfigureControllers() override;

    /// Persists the auto-map toggle into UISettings.
    void ApplyConfiguration();

signals:
    /// Emitted around the modal Advanced editor so the auto-map re-sync timer can be paused while
    /// the user edits bindings manually, then resumed afterwards.
    void AdvancedEditorVisibilityChanged(bool open);

private:
    void RefreshStatus();
    void OpenAdvanced();

    Core::System& system;
    InputCommon::InputSubsystem* input_subsystem;

    QCheckBox* auto_map_checkbox{};
    QPushButton* advanced_button{};
    std::array<QLabel*, 8> status_labels{};
    QLabel* status_empty_label{};
    QTimer* refresh_timer{};
};
