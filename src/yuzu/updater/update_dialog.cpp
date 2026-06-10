// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <qdesktopservices.h>
#include "common/logging.h"
#include "common/scm_rev.h"
#include "qt_common/abstract/frontend.h"
#include "qt_common/abstract/progress.h"
#include "ui_update_dialog.h"
#include "update_dialog.h"

#include "common/httplib.h"

#ifdef YUZU_BUNDLED_OPENSSL
#include <openssl/cert.h>
#endif

#include <QDesktopServices>

#undef GetSaveFileName

UpdateDialog::UpdateDialog(const Common::Net::Release& release, QWidget* parent)
    : QDialog(parent), ui(new Ui::UpdateDialog) {
    ui->setupUi(this);

    ui->version->setText(
        tr("%1 is available for download.").arg(QString::fromStdString(release.title)));
    ui->url->setText(
        tr("<a href=\"%1\">View release page</a>").arg(QString::fromStdString(release.html_url)));

    std::string text{release.body};
    if (auto pos = text.find("# Packages"); pos != std::string::npos) {
        text = text.substr(0, pos);
    }

    ui->body->setMarkdown(QString::fromStdString(text));

    // TODO(crueter): Find a way to set default
    const auto assets = release.GetPlatformAssets();

    if (assets.empty()) {
        ui->groupBox->setHidden(true);
        connect(this, &QDialog::accepted, this, [release]() {
            QDesktopServices::openUrl(QUrl{QString::fromStdString(release.html_url)});
        });
    } else if (assets.size() == 1) {
        ui->groupBox->setHidden(true);
        m_asset = assets[0];

        connect(this, &QDialog::accepted, this, &UpdateDialog::Download);
    } else {
        u32 i = 0;
        for (const Common::Net::Asset& a : assets) {
            QRadioButton* r = new QRadioButton(tr(a.name.c_str()), this);
            connect(r, &QRadioButton::toggled, this, [a, this](bool checked) {
                if (checked)
                    m_asset = a;
            });

            if (i == 0)
                r->setChecked(true);
            ++i;

            ui->radioButtons->addWidget(r);
        }

        connect(this, &QDialog::accepted, this, &UpdateDialog::Download);
    }
}

UpdateDialog::~UpdateDialog() {
    delete ui;
}

bool UpdateDialog::DownloadAssetTo(const QString& dest_path, bool require_verified_digest) {
    QSaveFile file(dest_path);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        LOG_WARNING(Frontend, "Could not open file {}", dest_path.toStdString());
        QtCommon::Frontend::Critical(tr("Failed to save file"),
                                     tr("Could not open file %1 for writing.").arg(dest_path));
        return false;
    }

    // TODO(crueter): Move to net.cpp
    // A short timeout is right for connecting, but the asset is hundreds of MB - use a much longer
    // stall timeout for the transfer so a brief hiccup (Wi-Fi handoff, throttling) does not discard
    // the whole download.
    constexpr std::size_t connect_timeout_seconds = 15;
    constexpr std::size_t transfer_timeout_seconds = 120;

    // The asset's download URL is an absolute URL that may live on a different host than the API
    // (GitHub serves release assets from github.com and then redirects to its CDN, not from
    // api.github.com). Derive the client host + request path from the asset URL itself rather than
    // from the API host, and follow redirects so the CDN hand-off works.
    std::string download_host = m_asset.url;
    std::string download_path = m_asset.path;
    if (const auto scheme_end = download_path.find("://"); scheme_end != std::string::npos) {
        const auto host_start = scheme_end + 3;
        if (const auto path_start = download_path.find('/', host_start);
            path_start != std::string::npos) {
            download_host = download_path.substr(0, path_start);
            download_path = download_path.substr(path_start);
        }
    }

    // Refuse a non-HTTPS download URL. TLS is the trust anchor for the transfer (alongside the
    // SHA-256 check below); never fetch the update in cleartext.
    if (download_host.rfind("https://", 0) != 0) {
        LOG_ERROR(Frontend, "Refusing non-HTTPS update URL: {}", m_asset.path);
        QtCommon::Frontend::Critical(
            tr("Failed to download file"),
            tr("Refusing to download the update over an insecure (non-HTTPS) connection."));
        return false;
    }

    std::unique_ptr<httplib::Client> client = std::make_unique<httplib::Client>(download_host);
    client->set_follow_location(true);
    client->set_connection_timeout(connect_timeout_seconds);
    client->set_read_timeout(transfer_timeout_seconds);
    client->set_write_timeout(transfer_timeout_seconds);

    // GitHub rejects requests without a User-Agent (HTTP 403); set one as a default header so it is
    // sent on the initial request and any followed redirect to the asset CDN.
    client->set_default_headers(
        {{"User-Agent", std::string("VibeEden/") + Common::g_build_version}});

#ifdef YUZU_BUNDLED_OPENSSL
    client->load_ca_cert_store(kCert, sizeof(kCert));
#elif defined(__linux__)
    // The AppImage bundles its own OpenSSL whose compiled-in cert directory (/usr/lib/ssl) does not
    // exist on SteamOS, so without this the download's TLS handshake fails to verify. Point the
    // client at the host CA bundle from a list of well-known locations.
    for (const char* ca : {"/etc/ssl/certs/ca-certificates.crt", "/etc/pki/tls/certs/ca-bundle.crt",
                           "/etc/ssl/ca-bundle.pem", "/etc/ssl/cert.pem",
                           "/etc/ca-certificates/extracted/tls-ca-bundle.pem"}) {
        if (QFile::exists(QString::fromLatin1(ca))) {
            client->set_ca_cert_path(ca);
            break;
        }
    }
#endif

    auto progress =
        QtCommon::Frontend::newProgressDialog(tr("Downloading..."), tr("Cancel"), 0, 100);
    progress->show();

    QGuiApplication::processEvents();

    auto progress_callback = [&](size_t processed_size, size_t total_size) {
        QGuiApplication::processEvents();
        if (total_size > 0) {
            progress->setValue(static_cast<int>((processed_size * 100) / total_size));
        }
        return !progress->wasCanceled();
    };

    auto content_receiver = [&file, &dest_path](const char* t_data, size_t data_length) -> bool {
        if (file.write(t_data, data_length) == -1) {
            LOG_WARNING(Frontend, "Could not write {} bytes to file {}", data_length,
                        dest_path.toStdString());
            return false;
        }
        return true;
    };

    auto result = client->Get(download_path, content_receiver, progress_callback);
    progress->close();

    if (!result) {
        LOG_ERROR(Frontend, "GET to {}{} returned null", download_host, download_path);
        QtCommon::Frontend::Critical(tr("Failed to download file"),
                                     tr("Could not download the update (no response)."));
        return false;
    }

    const auto& response = result.value();
    if (response.status >= 400) {
        LOG_ERROR(Frontend, "GET to {}{} returned error status code: {}", download_host,
                  download_path, response.status);
        QtCommon::Frontend::Critical(
            tr("Failed to download file"),
            tr("Download failed (HTTP %1).").arg(QString::number(response.status)));
        return false;
    }

    if (!file.commit()) {
        LOG_WARNING(Frontend, "Could not commit to file {}", dest_path.toStdString());
        QtCommon::Frontend::Critical(tr("Failed to save file"),
                                     tr("Could not commit to file %1.").arg(dest_path));
        return false;
    }

    // Verify integrity before the caller does anything with the file (the AppImage path makes it
    // executable and runs it). On mismatch the file is discarded.
    if (!VerifyDownloadIntegrity(dest_path, require_verified_digest)) {
        QFile::remove(dest_path);
        return false;
    }

    return true;
}

bool UpdateDialog::VerifyDownloadIntegrity(const QString& path, bool require_digest) {
    if (m_asset.sha256.empty()) {
        // The host did not advertise a digest (older release / non-GitHub host).
        if (require_digest) {
            // Refuse to auto-install (chmod +x + exec) something we cannot verify.
            LOG_ERROR(Frontend, "No SHA-256 digest for {}; refusing unverified auto-install",
                      m_asset.filename);
            QtCommon::Frontend::Critical(
                tr("Update verification unavailable"),
                tr("This release does not provide a checksum, so the update cannot be verified and "
                   "will not be installed automatically. Please update manually from the release "
                   "page."));
            return false;
        }
        LOG_WARNING(Frontend, "No SHA-256 digest for {}; skipping integrity check",
                    m_asset.filename);
        return true;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(Frontend, "Could not reopen {} to verify", path.toStdString());
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        LOG_ERROR(Frontend, "Could not hash {}", path.toStdString());
        return false;
    }

    const QString actual = QString::fromLatin1(hash.result().toHex());
    const QString expected = QString::fromStdString(m_asset.sha256);
    if (actual.compare(expected, Qt::CaseInsensitive) != 0) {
        LOG_ERROR(Frontend, "SHA-256 mismatch for {}: expected {}, got {}", m_asset.filename,
                  m_asset.sha256, actual.toStdString());
        QtCommon::Frontend::Critical(
            tr("Update verification failed"),
            tr("The downloaded update did not match its expected checksum and was discarded. "
               "Please try again later."));
        return false;
    }

    LOG_INFO(Frontend, "Verified SHA-256 of {}", m_asset.filename);
    return true;
}

void UpdateDialog::SelfInstallAppImage(const QString& appimage_path) {
    // Stage the download + helper in a fresh, private (0700) directory with an unpredictable name,
    // so another local user cannot pre-create or race the files (no predictable /tmp/...-<pid>).
    QTemporaryDir staging;
    if (!staging.isValid()) {
        QtCommon::Frontend::Critical(tr("Update failed"),
                                     tr("Could not create a temporary directory for the update."));
        return;
    }
    staging.setAutoRemove(false); // the detached helper removes it after applying the update.
    const QString staging_path = staging.path();
    const QString new_image = staging_path % QStringLiteral("/Eden.AppImage");
    const QString script_path = staging_path % QStringLiteral("/apply-update.sh");

    if (!DownloadAssetTo(new_image, /*require_verified_digest=*/true)) {
        QDir(staging_path).removeRecursively();
        return; // DownloadAssetTo already reported the error.
    }

    // Present a single, explicit "Restart and apply" action (plus a "Later" escape) instead of a
    // vague yes/no prompt.
    QMessageBox box(QMessageBox::Information, tr("Update downloaded"),
                    tr("The update has been downloaded. Restart VibeEden now to apply it."),
                    QMessageBox::NoButton, this);
    QPushButton* const restart_button =
        box.addButton(tr("Restart and apply"), QMessageBox::AcceptRole);
    box.addButton(tr("Later"), QMessageBox::RejectRole);
    box.setDefaultButton(restart_button);
    box.exec();
    if (box.clickedButton() != restart_button) {
        QDir(staging_path).removeRecursively();
        return;
    }

    // Detached helper. NEW/TARGET/PID/DIR are passed as positional arguments (never interpolated
    // into the script body) so a path containing shell metacharacters cannot inject commands. It
    // waits for this process to exit; if it is still alive after the wait it aborts (leaving the
    // running app untouched - there is nothing to relaunch). Once it has exited it swaps the image
    // and then always relaunches the target, so even a failed swap still brings the app back, then
    // cleans up.
    const QString script = QStringLiteral(
        "#!/bin/bash\n"
        "NEW=\"$1\"; TARGET=\"$2\"; PID=\"$3\"; DIR=\"$4\"\n"
        "for i in $(seq 1 60); do kill -0 \"$PID\" 2>/dev/null || break; sleep 0.5; done\n"
        "if kill -0 \"$PID\" 2>/dev/null; then rm -rf \"$DIR\"; exit 0; fi\n"
        "chmod +x \"$NEW\" 2>/dev/null\n"
        "mv -f \"$NEW\" \"$TARGET\" 2>/dev/null && chmod +x \"$TARGET\" 2>/dev/null\n"
        "nohup \"$TARGET\" >/dev/null 2>&1 &\n"
        "disown\n"
        "rm -rf \"$DIR\"\n");

    QFile f(script_path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QtCommon::Frontend::Critical(tr("Update failed"),
                                     tr("Could not write the update helper script."));
        QDir(staging_path).removeRecursively();
        return;
    }
    f.write(script.toUtf8());
    f.close();
    QFile::setPermissions(script_path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

    const QString pid_str = QString::number(QCoreApplication::applicationPid());
    if (!QProcess::startDetached(
            QStringLiteral("/bin/bash"),
            {script_path, new_image, appimage_path, pid_str, staging_path})) {
        QtCommon::Frontend::Critical(tr("Update failed"),
                                     tr("Could not start the update helper."));
        QDir(staging_path).removeRecursively();
        return;
    }

    accept();
    qApp->quit();
}

void UpdateDialog::Download() {
#if defined(__linux__)
    // When running from an AppImage, do an in-place self-install + relaunch instead of saving the
    // file somewhere and asking the user to install it by hand.
    const QString appimage_path = qEnvironmentVariable("APPIMAGE");
    const bool asset_is_appimage = QString::fromStdString(m_asset.filename)
                                       .endsWith(QStringLiteral(".AppImage"), Qt::CaseInsensitive);
    if (!appimage_path.isEmpty() && asset_is_appimage) {
        // Only self-install in place when the AppImage's location is writable; otherwise fall
        // through to the manual "save as" path instead of a swap that would silently fail.
        if (QFileInfo(appimage_path).isWritable()) {
            SelfInstallAppImage(appimage_path);
            return;
        }
        QtCommon::Frontend::Information(
            tr("Manual update needed"),
            tr("VibeEden is running from a read-only location and cannot replace itself there. "
               "Choose where to save the new version, then move it into place."));
    }
#endif

    const auto filename = QtCommon::Frontend::GetSaveFileName(
        tr("New Version Location"),
        qApp->applicationDirPath() % QStringLiteral("/") % QString::fromStdString(m_asset.filename),
        tr("All Files (*.*)"));

    if (filename.isEmpty())
        return;

    if (!DownloadAssetTo(filename))
        return;

    // Download is complete. User may choose to open in the file manager.
    auto button =
        QtCommon::Frontend::Question(tr("Download Complete"),
                                     tr("Successfully downloaded %1. Would you like to open it?")
                                         .arg(QString::fromStdString(m_asset.filename)),
                                     QtCommon::Frontend::Yes | QtCommon::Frontend::No);

    if (button == QtCommon::Frontend::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    }
}
