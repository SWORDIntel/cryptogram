/*
 * CRYPTOGRAM Settings Activity
 * Military-Grade Encryption Settings for Telegram Android
 *
 * This file is part of CRYPTOGRAM Android
 * Licensed under GNU GPL v. 2 or later.
 */

package org.telegram.ui;

import android.content.Context;
import android.content.SharedPreferences;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.telegram.messenger.AndroidUtilities;
import org.telegram.messenger.LocaleController;
import org.telegram.messenger.MessagesController;
import org.telegram.messenger.R;
import org.telegram.messenger.SharedConfig;
import org.telegram.messenger.cryptogram.CryptogramNative;
import org.telegram.messenger.cryptogram.DoubleRatchet;
import org.telegram.messenger.cryptogram.MLSProtocol;
import org.telegram.ui.ActionBar.ActionBar;
import org.telegram.ui.ActionBar.AlertDialog;
import org.telegram.ui.ActionBar.BaseFragment;
import org.telegram.ui.ActionBar.Theme;
import org.telegram.ui.ActionBar.ThemeDescription;
import org.telegram.ui.Cells.HeaderCell;
import org.telegram.ui.Cells.ShadowSectionCell;
import org.telegram.ui.Cells.TextCheckCell;
import org.telegram.ui.Cells.TextInfoPrivacyCell;
import org.telegram.ui.Cells.TextSettingsCell;
import org.telegram.ui.Components.LayoutHelper;
import org.telegram.ui.Components.RecyclerListView;

import java.util.ArrayList;
import java.util.Map;

import org.telegram.messenger.FileLog;

public class CryptogramSettingsActivity extends BaseFragment {

    private RecyclerListView listView;
    private ListAdapter listAdapter;
    private LinearLayoutManager layoutManager;

    // Row indices
    private int rowCount;

    private int cryptogramHeaderRow;
    private int cryptogramStatusRow;
    private int cryptogramShadowRow;

    private int encryptionSectionRow;
    private int doubleRatchetRow;
    private int mlsProtocolRow;
    private int encryptionInfoRow;

    private int privacySectionRow;
    private int hideOnlineStatusRow;
    private int hideTypingIndicatorRow;
    private int hideReadReceiptsRow;
    private int privacyInfoRow;

    private int uiSectionRow;
    private int curatedStickersRow;
    private int uiInfoRow;

    // OPSEC & Security section
    private int opsecSectionRow;
    private int panicPasswordRow;
    private int antiForensicsRow;
    private int deadManSwitchRow;
    private int mediaMetadataRow;
    private int trafficPaddingRow;
    private int trafficObfuscationRow;
    private int dpiEvasionRow;
    private int dpiEvasionMethodRow;
    private int quantumLevelRow;
    private int threatDefenseRow;
    private int stylometryRow;
    private int utdRow;
    private int voiceMorphingRow;
    private int locationPrivacyRow;
    private int interfaceCamouflageRow;
    private int hardwareTetherRow;
    private int imapProtectionRow;
    private int opsecInfoRow;

    // OPSEC Presets section
    private int presetSectionRow;
    private int presetStandardRow;
    private int presetEnhancedRow;
    private int presetMaximumRow;
    private int presetInfoRow;

    private int advancedSectionRow;
    private int libraryVersionRow;
    private int featureStatusRow;
    private int advancedInfoRow;

    @Override
    public boolean onFragmentCreate() {
        super.onFragmentCreate();
        updateRows();
        return true;
    }

    @Override
    public View createView(Context context) {
        actionBar.setBackButtonImage(R.drawable.ic_ab_back);
        actionBar.setAllowOverlayTitle(true);
        actionBar.setTitle("CRYPTOGRAM");
        actionBar.setActionBarMenuOnItemClick(new ActionBar.ActionBarMenuOnItemClick() {
            @Override
            public void onItemClick(int id) {
                if (id == -1) {
                    finishFragment();
                }
            }
        });

        listAdapter = new ListAdapter(context);

        fragmentView = new FrameLayout(context);
        fragmentView.setBackgroundColor(Theme.getColor(Theme.key_windowBackgroundGray));
        FrameLayout frameLayout = (FrameLayout) fragmentView;

        listView = new RecyclerListView(context);
        listView.setVerticalScrollBarEnabled(false);
        listView.setLayoutManager(layoutManager = new LinearLayoutManager(context, LinearLayoutManager.VERTICAL, false));
        frameLayout.addView(listView, LayoutHelper.createFrame(LayoutHelper.MATCH_PARENT, LayoutHelper.MATCH_PARENT));
        listView.setAdapter(listAdapter);
        listView.setOnItemClickListener((view, position, x, y) -> {
            if (position == doubleRatchetRow) {
                SharedConfig.toggleCryptogramDoubleRatchet();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramDoubleRatchetEnabled);
                }
            } else if (position == mlsProtocolRow) {
                SharedConfig.toggleCryptogramMLS();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramMLSEnabled);
                }
            } else if (position == hideOnlineStatusRow) {
                SharedConfig.toggleCryptogramHideOnlineStatus();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramHideOnlineStatus);
                }
            } else if (position == hideTypingIndicatorRow) {
                SharedConfig.toggleCryptogramHideTypingIndicator();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramHideTypingIndicator);
                }
            } else if (position == hideReadReceiptsRow) {
                SharedConfig.toggleCryptogramHideReadReceipts();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramHideReadReceipts);
                }
            } else if (position == curatedStickersRow) {
                SharedConfig.toggleCryptogramCuratedStickers();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramCuratedStickersEnabled);
                }
            } else if (position == panicPasswordRow) {
                SharedConfig.toggleCryptogramPanicPassword();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramPanicPasswordEnabled);
                }
                if (SharedConfig.cryptogramPanicPasswordEnabled) {
                    showPanicPasswordInfoDialog();
                }
            } else if (position == antiForensicsRow) {
                SharedConfig.toggleCryptogramAntiForensics();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramAntiForensicsEnabled);
                }
            } else if (position == deadManSwitchRow) {
                SharedConfig.toggleCryptogramDeadManSwitch();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramDeadManSwitchEnabled);
                }
                if (SharedConfig.cryptogramDeadManSwitchEnabled) {
                    showDeadManSwitchInfoDialog();
                }
            } else if (position == mediaMetadataRow) {
                SharedConfig.toggleCryptogramMediaMetadataSpoofing();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramMediaMetadataSpoofingEnabled);
                }
            } else if (position == trafficPaddingRow) {
                SharedConfig.toggleCryptogramTrafficPadding();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramTrafficPaddingEnabled);
                }
            } else if (position == trafficObfuscationRow) {
                SharedConfig.toggleCryptogramTrafficObfuscation();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramTrafficObfuscationEnabled);
                }
            } else if (position == dpiEvasionRow) {
                SharedConfig.toggleCryptogramDpiEvasion();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramDpiEvasionEnabled);
                }
            } else if (position == dpiEvasionMethodRow) {
                showDpiEvasionMethodDialog();
            } else if (position == quantumLevelRow) {
                showQuantumLevelDialog();
            } else if (position == threatDefenseRow) {
                showThreatDefenseDialog();
            } else if (position == stylometryRow) {
                SharedConfig.toggleCryptogramStylometryShield();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramStylometryShieldEnabled);
                }
            } else if (position == utdRow) {
                SharedConfig.toggleCryptogramUtd();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramUtdEnabled);
                }
            } else if (position == voiceMorphingRow) {
                SharedConfig.toggleCryptogramVoiceMorphing();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramVoiceMorphingEnabled);
                }
            } else if (position == locationPrivacyRow) {
                SharedConfig.toggleCryptogramLocationPrivacy();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramLocationPrivacyEnabled);
                }
            } else if (position == interfaceCamouflageRow) {
                SharedConfig.toggleCryptogramInterfaceCamouflage();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramInterfaceCamouflageEnabled);
                }
            } else if (position == hardwareTetherRow) {
                SharedConfig.toggleCryptogramHardwareTether();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramHardwareTetherEnabled);
                }
            } else if (position == imapProtectionRow) {
                SharedConfig.toggleCryptogramImapProtection();
                if (view instanceof TextCheckCell) {
                    ((TextCheckCell) view).setChecked(SharedConfig.cryptogramImapProtectionEnabled);
                }
            } else if (position == presetStandardRow) {
                applyPreset(0);
            } else if (position == presetEnhancedRow) {
                applyPreset(1);
            } else if (position == presetMaximumRow) {
                applyPreset(2);
            } else if (position == featureStatusRow) {
                showFeatureStatusDialog();
            }
        });

        return fragmentView;
    }

    private void updateRows() {
        rowCount = 0;

        cryptogramHeaderRow = rowCount++;
        cryptogramStatusRow = rowCount++;
        cryptogramShadowRow = rowCount++;

        encryptionSectionRow = rowCount++;
        doubleRatchetRow = rowCount++;
        mlsProtocolRow = rowCount++;
        encryptionInfoRow = rowCount++;

        privacySectionRow = rowCount++;
        hideOnlineStatusRow = rowCount++;
        hideTypingIndicatorRow = rowCount++;
        hideReadReceiptsRow = rowCount++;
        privacyInfoRow = rowCount++;

        uiSectionRow = rowCount++;
        curatedStickersRow = rowCount++;
        uiInfoRow = rowCount++;

        opsecSectionRow = rowCount++;
        panicPasswordRow = rowCount++;
        antiForensicsRow = rowCount++;
        deadManSwitchRow = rowCount++;
        mediaMetadataRow = rowCount++;
        trafficPaddingRow = rowCount++;
        trafficObfuscationRow = rowCount++;
        dpiEvasionRow = rowCount++;
        dpiEvasionMethodRow = rowCount++;
        quantumLevelRow = rowCount++;
        threatDefenseRow = rowCount++;
        stylometryRow = rowCount++;
        utdRow = rowCount++;
        voiceMorphingRow = rowCount++;
        locationPrivacyRow = rowCount++;
        interfaceCamouflageRow = rowCount++;
        hardwareTetherRow = rowCount++;
        imapProtectionRow = rowCount++;
        opsecInfoRow = rowCount++;

        presetSectionRow = rowCount++;
        presetStandardRow = rowCount++;
        presetEnhancedRow = rowCount++;
        presetMaximumRow = rowCount++;
        presetInfoRow = rowCount++;

        advancedSectionRow = rowCount++;
        libraryVersionRow = rowCount++;
        featureStatusRow = rowCount++;
        advancedInfoRow = rowCount++;
    }

    private void showPanicPasswordInfoDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(getParentActivity());
        builder.setTitle("Panic Password");
        builder.setMessage("WARNING: When enabled, a secondary panic password can be entered at login to trigger IRREVERSIBLE data destruction:\n\n" +
                "• All local databases will be securely erased\n" +
                "• All encryption keys will be wiped from memory\n" +
                "• All session data will be destroyed\n\n" +
                "The panic password must be different from your main login password.");
        builder.setPositiveButton(LocaleController.getString("OK", R.string.OK), null);
        showDialog(builder.create());
    }

    private void showDeadManSwitchInfoDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(getParentActivity());
        builder.setTitle("Dead Man's Switch");
        builder.setMessage("Dead Man's Switch Activated\n\nYou must provide proof of activity every 60 minutes. " +
                "Failure to do so will trigger emergency data destruction and notify your recovery contacts.");
        builder.setPositiveButton(LocaleController.getString("OK", R.string.OK), null);
        showDialog(builder.create());
    }

    private void showDpiEvasionMethodDialog() {
        String[] methods = {"HTTPS Mimicry", "HTTP Tunneling", "DNS Tunneling", "Generic Fragmentation", "Auto (rotate)"};
        AlertDialog.Builder builder = new AlertDialog.Builder(getParentActivity());
        builder.setTitle("Evasion Method");
        builder.setItems(methods, (dialog, which) -> {
            SharedConfig.setCryptogramDpiEvasionMethod(which);
            if (listAdapter != null) {
                listAdapter.notifyItemChanged(dpiEvasionMethodRow);
            }
        });
        builder.setNegativeButton(LocaleController.getString("Cancel", R.string.Cancel), null);
        showDialog(builder.create());
    }

    private void showQuantumLevelDialog() {
        String[] levels = {"Level 1 (AES-128)", "Level 3 (AES-256)", "Level 5 (Advanced)"};
        int[] values = {128, 256, 384};
        AlertDialog.Builder builder = new AlertDialog.Builder(getParentActivity());
        builder.setTitle("Quantum Security Level");
        builder.setItems(levels, (dialog, which) -> {
            SharedConfig.setCryptogramQuantumSecurityLevel(values[which]);
            if (listAdapter != null) {
                listAdapter.notifyItemChanged(quantumLevelRow);
            }
        });
        builder.setNegativeButton(LocaleController.getString("Cancel", R.string.Cancel), null);
        showDialog(builder.create());
    }

    private void showThreatDefenseDialog() {
        String[] levels = {"Standard (Baseline)", "Enhanced (Advanced Obfuscation)", "Maximum (Extreme Countermeasures)"};
        AlertDialog.Builder builder = new AlertDialog.Builder(getParentActivity());
        builder.setTitle("Threat Protection Level");
        builder.setItems(levels, (dialog, which) -> {
            SharedConfig.setCryptogramThreatDefenseLevel(which);
            if (listAdapter != null) {
                listAdapter.notifyItemChanged(threatDefenseRow);
            }
        });
        builder.setNegativeButton(LocaleController.getString("Cancel", R.string.Cancel), null);
        showDialog(builder.create());
    }

    private void applyPreset(int level) {
        String presetName;
        if (level == 0) {
            presetName = "Standard";
            SharedConfig.setCryptogramThreatDefenseLevel(0);
            SharedConfig.cryptogramTrafficObfuscationEnabled = false;
            SharedConfig.cryptogramDpiEvasionEnabled = false;
            SharedConfig.cryptogramStylometryShieldEnabled = false;
            SharedConfig.cryptogramUtdEnabled = false;
            SharedConfig.cryptogramMediaMetadataSpoofingEnabled = false;
            SharedConfig.cryptogramTrafficPaddingEnabled = false;
            SharedConfig.cryptogramInterfaceCamouflageEnabled = false;
        } else if (level == 1) {
            presetName = "Enhanced";
            SharedConfig.setCryptogramThreatDefenseLevel(1);
            SharedConfig.cryptogramTrafficObfuscationEnabled = true;
            SharedConfig.cryptogramDpiEvasionEnabled = true;
            SharedConfig.cryptogramStylometryShieldEnabled = true;
            SharedConfig.cryptogramUtdEnabled = true;
            SharedConfig.cryptogramMediaMetadataSpoofingEnabled = true;
            SharedConfig.cryptogramTrafficPaddingEnabled = false;
            SharedConfig.cryptogramInterfaceCamouflageEnabled = false;
        } else {
            presetName = "Maximum";
            SharedConfig.setCryptogramThreatDefenseLevel(2);
            SharedConfig.cryptogramTrafficObfuscationEnabled = true;
            SharedConfig.cryptogramDpiEvasionEnabled = true;
            SharedConfig.cryptogramStylometryShieldEnabled = true;
            SharedConfig.cryptogramUtdEnabled = true;
            SharedConfig.cryptogramMediaMetadataSpoofingEnabled = true;
            SharedConfig.cryptogramTrafficPaddingEnabled = true;
            SharedConfig.cryptogramInterfaceCamouflageEnabled = true;
            SharedConfig.cryptogramAntiForensicsEnabled = true;
        }
        SharedPreferences preferences = org.telegram.messenger.MessagesController.getGlobalMainSettings();
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean("cryptogramTrafficObfuscation", SharedConfig.cryptogramTrafficObfuscationEnabled);
        editor.putBoolean("cryptogramDpiEvasion", SharedConfig.cryptogramDpiEvasionEnabled);
        editor.putBoolean("cryptogramStylometryShield", SharedConfig.cryptogramStylometryShieldEnabled);
        editor.putBoolean("cryptogramUtd", SharedConfig.cryptogramUtdEnabled);
        editor.putBoolean("cryptogramMediaMetadataSpoofing", SharedConfig.cryptogramMediaMetadataSpoofingEnabled);
        editor.putBoolean("cryptogramTrafficPadding", SharedConfig.cryptogramTrafficPaddingEnabled);
        editor.putBoolean("cryptogramInterfaceCamouflage", SharedConfig.cryptogramInterfaceCamouflageEnabled);
        editor.putBoolean("cryptogramAntiForensics", SharedConfig.cryptogramAntiForensicsEnabled);
        editor.apply();

        if (listAdapter != null) {
            listAdapter.notifyDataSetChanged();
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(getParentActivity());
        builder.setTitle("OPSEC Preset Applied");
        builder.setMessage("The " + presetName + " preset has been applied. Review the toggles below to see what changed.");
        builder.setPositiveButton(LocaleController.getString("OK", R.string.OK), null);
        showDialog(builder.create());
    }

    private void showFeatureStatusDialog() {
        Map<String, Boolean> status = CryptogramNative.INSTANCE.getFeatureStatus();
        StringBuilder message = new StringBuilder();
        message.append("CRYPTOGRAM Feature Status:\n\n");

        for (Map.Entry<String, Boolean> entry : status.entrySet()) {
            String icon = entry.getValue() ? "✓" : "✗";
            message.append(icon).append(" ").append(entry.getKey()).append("\n");
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(getParentActivity());
        builder.setTitle("Feature Status");
        builder.setMessage(message.toString());
        builder.setPositiveButton(LocaleController.getString("OK", R.string.OK), null);
        showDialog(builder.create());
    }

    private class ListAdapter extends RecyclerListView.SelectionAdapter {

        private Context mContext;

        public ListAdapter(Context context) {
            mContext = context;
        }

        @Override
        public int getItemCount() {
            return rowCount;
        }

        @Override
        public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
            switch (holder.getItemViewType()) {
                case 0: {
                    // Shadow
                    break;
                }
                case 1: {
                    // TextSettingsCell
                    TextSettingsCell textCell = (TextSettingsCell) holder.itemView;
                    if (position == cryptogramStatusRow) {
                        Map<String, Boolean> status = CryptogramNative.INSTANCE.getFeatureStatus();
                        boolean nativeReady = Boolean.TRUE.equals(status.get("Native Library"));
                        boolean ratchetReady = Boolean.TRUE.equals(status.get("Double Ratchet"));
                        boolean mlsReady = Boolean.TRUE.equals(status.get("MLS Protocol"));
                        String value;
                        if (nativeReady && ratchetReady && mlsReady) {
                            value = "🔐 Local self-checks passed";
                        } else if (nativeReady) {
                            value = "⚠️ Native loaded, partial checks";
                        } else {
                            value = "⚠️ Native library unavailable";
                        }
                        textCell.setTextAndValue("Status", value, false);
                    } else if (position == libraryVersionRow) {
                        String version = CryptogramNative.INSTANCE.getVersion();
                        textCell.setTextAndValue("Library Version", version, false);
                    } else if (position == featureStatusRow) {
                        textCell.setText("Feature Status", true);
                    } else if (position == dpiEvasionMethodRow) {
                        String[] methods = {"HTTPS Mimicry", "HTTP Tunneling", "DNS Tunneling", "Generic Fragmentation", "Auto (rotate)"};
                        int m = SharedConfig.cryptogramDpiEvasionMethod;
                        String methodStr = (m >= 0 && m < methods.length) ? methods[m] : "Unknown";
                        textCell.setTextAndValue("Evasion Method", methodStr, true);
                    } else if (position == quantumLevelRow) {
                        int lvl = SharedConfig.cryptogramQuantumSecurityLevel;
                        String lvlStr;
                        if (lvl <= 128) lvlStr = "Level 1 (AES-128)";
                        else if (lvl <= 256) lvlStr = "Level 3 (AES-256)";
                        else lvlStr = "Level 5 (Advanced)";
                        textCell.setTextAndValue("Quantum Security Level", lvlStr, true);
                    } else if (position == threatDefenseRow) {
                        int td = SharedConfig.cryptogramThreatDefenseLevel;
                        String tdStr;
                        if (td == 0) tdStr = "Standard";
                        else if (td == 1) tdStr = "Enhanced";
                        else tdStr = "Maximum";
                        textCell.setTextAndValue("Threat Defense Level", tdStr, true);
                    } else if (position == presetStandardRow) {
                        textCell.setText("Standard Preset", true);
                    } else if (position == presetEnhancedRow) {
                        textCell.setText("Enhanced Preset", true);
                    } else if (position == presetMaximumRow) {
                        textCell.setText("Maximum Preset", true);
                    }
                    break;
                }
                case 2: {
                    // HeaderCell
                    HeaderCell headerCell = (HeaderCell) holder.itemView;
                    if (position == cryptogramHeaderRow) {
                        headerCell.setText("🔐 CRYPTOGRAM");
                    } else if (position == encryptionSectionRow) {
                        headerCell.setText("Encryption Protocols");
                    } else if (position == privacySectionRow) {
                        headerCell.setText("Privacy Settings");
                    } else if (position == uiSectionRow) {
                        headerCell.setText("UI/UX Preferences");
                    } else if (position == opsecSectionRow) {
                        headerCell.setText("OPSEC & Security");
                    } else if (position == presetSectionRow) {
                        headerCell.setText("Quick OPSEC Presets");
                    } else if (position == advancedSectionRow) {
                        headerCell.setText("Advanced");
                    }
                    break;
                }
                case 3: {
                    // TextCheckCell
                    TextCheckCell textCheckCell = (TextCheckCell) holder.itemView;
                    if (position == doubleRatchetRow) {
                        textCheckCell.setTextAndCheck("Double Ratchet (Signal Protocol)",
                            SharedConfig.cryptogramDoubleRatchetEnabled, true);
                    } else if (position == mlsProtocolRow) {
                        textCheckCell.setTextAndCheck("MLS for Groups (RFC 9420)",
                            SharedConfig.cryptogramMLSEnabled, true);
                    } else if (position == hideOnlineStatusRow) {
                        textCheckCell.setTextAndCheck("Hide Online Status",
                            SharedConfig.cryptogramHideOnlineStatus, true);
                    } else if (position == hideTypingIndicatorRow) {
                        textCheckCell.setTextAndCheck("Hide Typing Indicator",
                            SharedConfig.cryptogramHideTypingIndicator, true);
                    } else if (position == hideReadReceiptsRow) {
                        textCheckCell.setTextAndCheck("Hide Read Receipts",
                            SharedConfig.cryptogramHideReadReceipts, false);
                    } else if (position == curatedStickersRow) {
                        textCheckCell.setTextAndCheck("Curated Stickers",
                            SharedConfig.cryptogramCuratedStickersEnabled, true);
                    } else if (position == panicPasswordRow) {
                        textCheckCell.setTextAndCheck("Panic Password (Secure Erase)",
                            SharedConfig.cryptogramPanicPasswordEnabled, true);
                    } else if (position == antiForensicsRow) {
                        textCheckCell.setTextAndCheck("Anti-Forensics (Evidence Destruction)",
                            SharedConfig.cryptogramAntiForensicsEnabled, true);
                    } else if (position == deadManSwitchRow) {
                        textCheckCell.setTextAndCheck("Dead Man's Switch (Failsafe)",
                            SharedConfig.cryptogramDeadManSwitchEnabled, true);
                    } else if (position == mediaMetadataRow) {
                        textCheckCell.setTextAndCheck("Spoof Media Metadata (EXIF/GPS)",
                            SharedConfig.cryptogramMediaMetadataSpoofingEnabled, true);
                    } else if (position == trafficPaddingRow) {
                        textCheckCell.setTextAndCheck("Network Traffic Padding",
                            SharedConfig.cryptogramTrafficPaddingEnabled, true);
                    } else if (position == trafficObfuscationRow) {
                        textCheckCell.setTextAndCheck("Advanced Traffic Obfuscation",
                            SharedConfig.cryptogramTrafficObfuscationEnabled, true);
                    } else if (position == dpiEvasionRow) {
                        textCheckCell.setTextAndCheck("DPI Evasion",
                            SharedConfig.cryptogramDpiEvasionEnabled, true);
                    } else if (position == stylometryRow) {
                        textCheckCell.setTextAndCheck("Stylometry Shield (Writing Privacy)",
                            SharedConfig.cryptogramStylometryShieldEnabled, true);
                    } else if (position == utdRow) {
                        textCheckCell.setTextAndCheck("Universal Threat Detector",
                            SharedConfig.cryptogramUtdEnabled, true);
                    } else if (position == voiceMorphingRow) {
                        textCheckCell.setTextAndCheck("Voice Morphing (AI Anonymization)",
                            SharedConfig.cryptogramVoiceMorphingEnabled, true);
                    } else if (position == locationPrivacyRow) {
                        textCheckCell.setTextAndCheck("Location Privacy (Randomization)",
                            SharedConfig.cryptogramLocationPrivacyEnabled, true);
                    } else if (position == interfaceCamouflageRow) {
                        textCheckCell.setTextAndCheck("Interface Camouflage",
                            SharedConfig.cryptogramInterfaceCamouflageEnabled, true);
                    } else if (position == hardwareTetherRow) {
                        textCheckCell.setTextAndCheck("Hardware Kill Switch (Tether)",
                            SharedConfig.cryptogramHardwareTetherEnabled, true);
                    } else if (position == imapProtectionRow) {
                        textCheckCell.setTextAndCheck("IMAP & Protocol Data Protection",
                            SharedConfig.cryptogramImapProtectionEnabled, false);
                    }
                    break;
                }
                case 4: {
                    // TextInfoPrivacyCell
                    TextInfoPrivacyCell cell = (TextInfoPrivacyCell) holder.itemView;
                    if (position == cryptogramShadowRow) {
                        cell.setText("CRYPTOGRAM adds native cryptography and privacy controls to Telegram. Feature availability below reflects local native self-checks and runtime wiring, not a full external security audit.");
                    } else if (position == encryptionInfoRow) {
                        cell.setText("• Double Ratchet: End-to-end encryption for 1-on-1 chats using Signal Protocol (X25519, Ed25519, AES-256-GCM)\n\n" +
                                    "• MLS Protocol: Scalable group encryption with TreeKEM (O(log n) operations, supports 10,000+ members)\n\n" +
                                    "Both protocols are exposed through the CRYPTOGRAM native layer. Use Feature Status to verify the local runtime path before relying on them.");
                    } else if (position == privacyInfoRow) {
                        cell.setText("• Hide Online Status: Prevents sending your online/offline status to other users\n\n" +
                                    "• Hide Typing Indicator: Stops sending typing notifications when you compose messages\n\n" +
                                    "• Hide Read Receipts: Prevents sending read confirmations (double ticks) when you view messages\n\n" +
                                    "These settings enhance your privacy by controlling what activity information is shared.");
                    } else if (position == uiInfoRow) {
                        cell.setText("Curated Stickers: Show a favorites section at the top of your sticker picker with your most-used sticker sets for quick access. All stickers remain searchable.\n\n" +
                                    "Long-press any sticker to add or remove its set from your curated favorites.");
                    } else if (position == opsecInfoRow) {
                        cell.setText("OPSEC features provide advanced security for extreme threat environments. These include panic password for emergency data destruction, anti-forensics for evidence wiping, DPI evasion for defeating deep packet inspection, and post-quantum cryptography for future-proof encryption.\n\n" +
                                    "Most features are experimental and should be validated on your target device before relying on them.");
                    } else if (position == presetInfoRow) {
                        cell.setText("Quick-apply OPSEC preset profiles. Standard provides baseline protection. Enhanced adds traffic obfuscation and stylometry. Maximum enables all countermeasures including anti-forensics and interface camouflage.");
                    } else if (position == advancedInfoRow) {
                        cell.setText("CRYPTOGRAM performs cryptographic operations locally through the native library. Feature Status runs local self-checks for the current build; device-level interoperability and broader security validation still depend on your target environment.\n\n" +
                                    "For technical details, see the CRYPTOGRAM documentation.");
                    }
                    break;
                }
            }
        }

        @Override
        public boolean isEnabled(RecyclerView.ViewHolder holder) {
            int type = holder.getItemViewType();
            return type == 1 || type == 3;
        }

        @Override
        public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view;
            switch (viewType) {
                case 0:
                    view = new ShadowSectionCell(mContext);
                    break;
                case 1:
                    view = new TextSettingsCell(mContext);
                    view.setBackgroundColor(Theme.getColor(Theme.key_windowBackgroundWhite));
                    break;
                case 2:
                    view = new HeaderCell(mContext);
                    view.setBackgroundColor(Theme.getColor(Theme.key_windowBackgroundWhite));
                    break;
                case 3:
                    view = new TextCheckCell(mContext);
                    view.setBackgroundColor(Theme.getColor(Theme.key_windowBackgroundWhite));
                    break;
                case 4:
                default:
                    view = new TextInfoPrivacyCell(mContext);
                    break;
            }
            view.setLayoutParams(new RecyclerView.LayoutParams(RecyclerView.LayoutParams.MATCH_PARENT, RecyclerView.LayoutParams.WRAP_CONTENT));
            return new RecyclerListView.Holder(view);
        }

        @Override
        public int getItemViewType(int position) {
            if (position == cryptogramShadowRow) {
                return 0; // Shadow
            } else if (position == cryptogramStatusRow || position == libraryVersionRow || position == featureStatusRow
                    || position == dpiEvasionMethodRow || position == quantumLevelRow || position == threatDefenseRow
                    || position == presetStandardRow || position == presetEnhancedRow || position == presetMaximumRow) {
                return 1; // TextSettingsCell
            } else if (position == cryptogramHeaderRow || position == encryptionSectionRow || position == privacySectionRow
                    || position == uiSectionRow || position == opsecSectionRow || position == presetSectionRow
                    || position == advancedSectionRow) {
                return 2; // HeaderCell
            } else if (position == doubleRatchetRow || position == mlsProtocolRow || position == hideOnlineStatusRow
                    || position == hideTypingIndicatorRow || position == hideReadReceiptsRow || position == curatedStickersRow
                    || position == panicPasswordRow || position == antiForensicsRow || position == deadManSwitchRow
                    || position == mediaMetadataRow || position == trafficPaddingRow || position == trafficObfuscationRow
                    || position == dpiEvasionRow || position == stylometryRow || position == utdRow
                    || position == voiceMorphingRow || position == locationPrivacyRow || position == interfaceCamouflageRow
                    || position == hardwareTetherRow || position == imapProtectionRow) {
                return 3; // TextCheckCell
            } else if (position == encryptionInfoRow || position == privacyInfoRow || position == uiInfoRow
                    || position == opsecInfoRow || position == presetInfoRow || position == advancedInfoRow) {
                return 4; // TextInfoPrivacyCell
            }
            return 0;
        }
    }

    @Override
    public ArrayList<ThemeDescription> getThemeDescriptions() {
        ArrayList<ThemeDescription> themeDescriptions = new ArrayList<>();

        themeDescriptions.add(new ThemeDescription(listView, ThemeDescription.FLAG_CELLBACKGROUNDCOLOR, new Class[]{TextSettingsCell.class, TextCheckCell.class, HeaderCell.class}, null, null, null, Theme.key_windowBackgroundWhite));
        themeDescriptions.add(new ThemeDescription(fragmentView, ThemeDescription.FLAG_BACKGROUND, null, null, null, null, Theme.key_windowBackgroundGray));

        themeDescriptions.add(new ThemeDescription(actionBar, ThemeDescription.FLAG_BACKGROUND, null, null, null, null, Theme.key_actionBarDefault));
        themeDescriptions.add(new ThemeDescription(listView, ThemeDescription.FLAG_LISTGLOWCOLOR, null, null, null, null, Theme.key_actionBarDefault));
        themeDescriptions.add(new ThemeDescription(actionBar, ThemeDescription.FLAG_AB_ITEMSCOLOR, null, null, null, null, Theme.key_actionBarDefaultIcon));
        themeDescriptions.add(new ThemeDescription(actionBar, ThemeDescription.FLAG_AB_TITLECOLOR, null, null, null, null, Theme.key_actionBarDefaultTitle));
        themeDescriptions.add(new ThemeDescription(actionBar, ThemeDescription.FLAG_AB_SELECTORCOLOR, null, null, null, null, Theme.key_actionBarDefaultSelector));

        themeDescriptions.add(new ThemeDescription(listView, ThemeDescription.FLAG_SELECTOR, null, null, null, null, Theme.key_listSelector));

        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{View.class}, Theme.dividerPaint, null, null, Theme.key_divider));

        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{HeaderCell.class}, new String[]{"textView"}, null, null, null, Theme.key_windowBackgroundWhiteBlueHeader));

        themeDescriptions.add(new ThemeDescription(listView, ThemeDescription.FLAG_BACKGROUNDFILTER, new Class[]{ShadowSectionCell.class}, null, null, null, Theme.key_windowBackgroundGrayShadow));

        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{TextSettingsCell.class}, new String[]{"textView"}, null, null, null, Theme.key_windowBackgroundWhiteBlackText));
        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{TextSettingsCell.class}, new String[]{"valueTextView"}, null, null, null, Theme.key_windowBackgroundWhiteValueText));

        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{TextCheckCell.class}, new String[]{"textView"}, null, null, null, Theme.key_windowBackgroundWhiteBlackText));
        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{TextCheckCell.class}, new String[]{"checkBox"}, null, null, null, Theme.key_switchTrack));
        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{TextCheckCell.class}, new String[]{"checkBox"}, null, null, null, Theme.key_switchTrackChecked));

        themeDescriptions.add(new ThemeDescription(listView, ThemeDescription.FLAG_BACKGROUNDFILTER, new Class[]{TextInfoPrivacyCell.class}, null, null, null, Theme.key_windowBackgroundGrayShadow));
        themeDescriptions.add(new ThemeDescription(listView, 0, new Class[]{TextInfoPrivacyCell.class}, new String[]{"textView"}, null, null, null, Theme.key_windowBackgroundWhiteGrayText4));

        return themeDescriptions;
    }
}
