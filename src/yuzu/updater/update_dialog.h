// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>
#include "common/net/net.h"

class QRadioButton;
namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog {
    Q_OBJECT

public:
    explicit UpdateDialog(const Common::Net::Release& release, QWidget* parent = nullptr);
    ~UpdateDialog();

private slots:
    void Download();

private:
    // Streams the selected asset to dest_path with a progress dialog, verifying its SHA-256 against
    // the host-provided digest. When require_verified_digest is true (the auto-execute install
    // path), a missing digest is treated as a failure. Returns success.
    bool DownloadAssetTo(const QString& dest_path, bool require_verified_digest = false);

    // Verifies the file at path against m_asset.sha256. Returns true on match. When the digest is
    // unknown: returns false if require_digest (refuse to trust an unverifiable auto-install),
    // otherwise true (cannot verify). Returns false on mismatch or read error.
    bool VerifyDownloadIntegrity(const QString& path, bool require_digest);

    // Linux/AppImage self-install: downloads the new AppImage, then on confirmation writes a
    // detached helper that waits for this process to exit, swaps $APPIMAGE, and relaunches.
    void SelfInstallAppImage(const QString& appimage_path);

    Ui::UpdateDialog* ui;
    QList<QRadioButton*> m_buttons;
    Common::Net::Asset m_asset;
};
