/*
Copyright (c) 2026 Matthew H. Reilly (kb1vc)
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

/**
 * @file SoDaCreateConfig.cxx
 *
 * @brief Generate a fresh SoDaRadio band-map configuration file.
 *
 * Writes a QSettings INI file in the format SoDaRadio's MainWindow expects
 * (Bands array under the Radio group), named <MODEL>_SoDaRadioQT.conf
 * in the current working directory.  Move the result to
 * ~/.config/kb1vc.org/ to have it picked up by
 * 'SoDaRadio --radio <model_name>'.
 *
 * If the target already exists the new file is written with a ".new"
 * suffix and the user is warned so the existing file is preserved.
 *
 * Optional transverter parameters install a TVMode/TVLOFreq entry on
 * any band that falls below the down-converter low limit (HE < ll_freq)
 * or above the up-converter high limit (LE > hl_freq), per the spec in
 * ConfigProgram.txt.
 */

#include <SoDa/Options.hxx>
#include <SoDa/Format.hxx>

#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QString>

#include <iostream>
#include <string>
#include <vector>

namespace {

  struct BandSpec {
    const char * name;
    double low_edge_mhz;
    double high_edge_mhz;
    bool   tx_enabled;
  };

  // Band table from ConfigProgram.txt -- indices assigned 1..N in order.
  const std::vector<BandSpec> kBands = {
    {"AM_BCast",   0.5,    1.7,    false},
    {"160m",       1.8,    2.0,    true},
    {"80m",        3.5,    4.0,    true},
    {"40m",        7.0,    7.3,    true},
    {"30m",        10.1,   10.15,  true},
    {"20m",        14.0,   14.35,  true},
    {"17m",        18.068, 18.168, true},
    {"15m",        21.0,   21.45,  true},
    {"12m",        24.89,  24.99,  true},
    {"10m",        28.0,   29.7,   true},
    {"6m",         50.0,   54.0,   true},
    {"2m",         144.0,  148.0,  true},
    {"1.25m",      219.0,  225.0,  true},
    {"70cm",       420.0,  450.0,  true},
    {"33cm",       920.0,  928.0,  true},
    {"23cm",       1240.0, 1300.0, true},
    {"WWV_10",     9.9,    10.1,   false},
    {"WX",         162.35, 162.60, false},
  };

  // Filename for the generated config.  The model name is uppercased so it
  // matches the lookup MainWindow does at startup
  // (QSettings("kb1vc.org", model_name.toUpper() + "_SoDaRadioQT")); the
  // user can move the resulting file to ~/.config/kb1vc.org/ to have it
  // picked up by 'SoDaRadio --radio <model_name>'.
  QString defaultConfigPath(const std::string & model_name)
  {
    QString model = QString::fromStdString(model_name).toUpper();
    return model + QStringLiteral("_SoDaRadioQT.conf");
  }

  // Populate one band entry under the current QSettings array index.
  void writeBandEntry(QSettings & settings,
                      int idx,
                      const BandSpec & b,
                      bool have_dcon, double dcon_freq, double ll_freq,
                      bool have_ucon, double ucon_freq, double hl_freq)
  {
    settings.setArrayIndex(idx);

    settings.setValue("Name",         QString::fromUtf8(b.name));
    settings.setValue("Index",        idx + 1);            // 1-based per spec
    settings.setValue("DefRXAnt",     QStringLiteral("RX"));
    settings.setValue("DefTXAnt",     QStringLiteral("TX"));
    settings.setValue("DefMode",      QStringLiteral("USB"));
    settings.setValue("MinFreq",      b.low_edge_mhz);
    settings.setValue("MaxFreq",      b.high_edge_mhz);

    double mid = 0.5 * (b.low_edge_mhz + b.high_edge_mhz);
    settings.setValue("LastRXFreq",   mid);
    settings.setValue("LastTXFreq",   mid);

    settings.setValue("TXEna",        b.tx_enabled);
    settings.setValue("FullDuplex",   false);
    settings.setValue("SatOffset",    0.0);
    settings.setValue("SatOffsetEna", false);

    // Default: no transverter.
    bool   tv_mode    = false;
    double tv_lo_freq = 0.0;
    double tv_lo_mult = 0.0;
    bool   tv_low_inj = false;

    if (have_dcon && (b.high_edge_mhz > ll_freq)) {
      tv_mode    = true;
      tv_lo_freq = dcon_freq;
      tv_lo_mult = 1.0;
      tv_low_inj = false;
    }
    else if (have_ucon && (b.low_edge_mhz < hl_freq)) {
      tv_mode    = true;
      tv_lo_freq = ucon_freq;
      tv_lo_mult = 1.0;
      tv_low_inj = true;
    }

    settings.setValue("TVMode",         tv_mode);
    settings.setValue("TVLOFreq",       tv_lo_freq);
    settings.setValue("TVLOMult",       tv_lo_mult);
    settings.setValue("TVLowInjection", tv_low_inj);
  }

} // namespace

int main(int argc, char ** argv)
{
  double dcon_freq = 0.0;
  double ll_freq   = 0.0;
  double ucon_freq = 0.0;
  double hl_freq   = 0.0;

  SoDa::Options cmd;
  cmd.addInfo(
    "SoDaCreateConfig <model_name> "
    "[--up-convert FREQ_MHz --low-limit FREQ_MHz] "
    "[--down-convert FREQ_MHz --high-limit FREQ_MHz]");
  cmd.add<double>(&ucon_freq, "up-convert", 'u', 0.0,
    "up-converter LO frequency in MHz (pair with --high-limit)");
  cmd.add<double>(&hl_freq, "high-limit", 'l', 0.0,
    "bands whose high edge is below this MHz get the down-converter LO");
  cmd.add<double>(&dcon_freq, "down-convert", 'd', 0.0,
    "down-converter LO frequency in MHz (pair with --low-limit)");
  cmd.add<double>(&ll_freq, "low-limit", 'h', 0.0,
    "bands whose low edge is above this MHz get the up-converter LO");
  bool help_flag;
  cmd.addP(&help_flag, "help", 'H', "Print this help message");
  cmd.addInfo(R"(
usage:
    SoDaCreateConfig [options] model_name

    Conversion is seen from the antenna side into a receiver. So
if we want to convert a 10MHz signal to where a Pluto SDR can "hear"
we might use a Ham-it-up converter with an LO at 125 MHz. Then we'd
say something like

    SoDaCreateConfig --up-convert 125.0 --high-limit 30.0 PLUTO

To create a band configuration file for a Pluto that up-converts from
the HF band to 125 to 155 MHz at the Pluto's RX port.
)");


  if (!cmd.parse(argc, argv)) {
    // Options prints its own --help / error message.
    return 0;
  }

  if(help_flag) {
    cmd.printHelp(std::cerr);
    return 0;
  }
  
  if (cmd.numPosArgs() < 1) {
    std::cerr << "Error: missing <model_name>.\n";
    cmd.printHelp(std::cerr);
    return 1;
  }

  std::string model_name = cmd.getPosArg(0);

  // Validate paired flags.
  bool dcon_p = cmd.isPresent("down-convert");
  bool ucon_p = cmd.isPresent("up-convert");
  
  bool hl_p   = cmd.isPresent("high-limit");
  bool ll_p   = cmd.isPresent("low-limit");
  
  if (ucon_p != hl_p) {
    std::cerr << "Warning: --up-convert and --high-limit must be given "
                 "together; ignoring up-converter settings.\n";
    ucon_p = hl_p = false;
  }
  if (dcon_p != ll_p) {
    std::cerr << "Warning: --down-convert and --low-limit must be given "
                 "together; ignoring down-converter settings.\n";
    dcon_p = ll_p = false;
  }
  bool have_dcon = dcon_p && ll_p;
  bool have_ucon = ucon_p && hl_p;

  // Compute the target path and decide whether to write a .new variant.
  QString target_path = defaultConfigPath(model_name);
  QString write_path  = target_path;
  if (QFile::exists(target_path)) {
    write_path = target_path + QStringLiteral(".new");
    std::cerr << SoDa::Format("Warning: %0 already exists; writing %1 instead.\n")
                    .addS(target_path.toStdString())
                    .addS(write_path.toStdString());
  }

  // Make sure the parent directory exists.
  QDir().mkpath(QFileInfo(write_path).absolutePath());

  // Wipe any stale .new from a previous run so the array isn't appended to.
  QFile::remove(write_path);

  QSettings settings(write_path, QSettings::IniFormat);
  settings.beginGroup("Radio");
  settings.beginWriteArray("Bands");
  for (int i = 0; i < static_cast<int>(kBands.size()); i++) {
    writeBandEntry(settings, i, kBands[i],
                   have_dcon, dcon_freq, ll_freq,
                   have_ucon, ucon_freq, hl_freq);
  }
  settings.endArray();
  settings.endGroup();
  settings.sync();

  if (settings.status() != QSettings::NoError) {
    std::cerr << SoDa::Format("Error: failed to write %0 (QSettings status = %1).\n")
                    .addS(write_path.toStdString())
                    .addI(static_cast<int>(settings.status()));
    return 2;
  }

  std::cout << SoDa::Format("Wrote %0 bands for radio model [%1] to %2\n")
                  .addI(static_cast<int>(kBands.size()))
                  .addS(model_name)
                  .addS(write_path.toStdString());
  return 0;
}
