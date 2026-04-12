# SeedSigner UI inventory for seedsigner-lvgl

Discovery inventory based on `SeedSigner/seedsigner` current source surface, with light historical grounding from the project’s still-present legacy structures (`views/*`, generic screen primitives, splash/screensaver code, screenshot generator hooks). This is primarily a design inventory, with an audit snapshot of what `seedsigner-lvgl` has already implemented.

## Scope and reading notes

### Audit snapshot: what `seedsigner-lvgl` now covers

As of current `main`, the LVGL port has working implementations for these screen families and support systems:
- menu/list shell
- warning/error/dire-warning family
- QR display
- keyboard text entry
- scan screen with external frame injection and mock detection
- settings menu and settings selection
- seed words display
- PSBT overview / math / detail screens
- startup splash and screensaver
- top-nav component, input profiles, SDL desktop runner, and screenshot regression tooling

That means this document should now be read as both:
- an inventory of the upstream SeedSigner UI surface, and
- a reference for choosing which remaining high-value families to port next

- Primary source areas reviewed:
  - `src/seedsigner/views/*.py`
  - `src/seedsigner/gui/screens/*.py`
  - `src/seedsigner/gui/components.py`
  - `src/seedsigner/gui/keyboard.py`
  - `src/seedsigner/views/screensaver.py`
  - `src/seedsigner/resources/{fonts,icons,img}`
- Focus is the current useful UI surface, but the document also notes generic/legacy primitives that still shape the app.
- Important distinction:
  - **Views** define flow/state transitions.
  - **Screens** define concrete rendered pages.
  - Some flows use **generic screens** (`ButtonListScreen`, `WarningScreen`, `QRDisplayScreen`, etc.) rather than a dedicated screen class.

## High-level UI structure

SeedSigner’s UI is built around a small-screen, hardware-button-first interaction model:

- 240x240 style square canvas assumptions are visible throughout the layout math.
- Top navigation bar is a persistent pattern.
- Core interaction modes:
  - menu/list navigation
  - large 2-up / 2x2 home buttons
  - text/seed/passphrase entry via on-screen keyboard + hardware buttons
  - QR scan and QR display
  - confirm/review/status/warning screens
- The product is organized around four main domains:
  - **Scan**
  - **Seeds**
  - **Tools**
  - **Settings**

## Visual language, assets, and data/display formats

### Core visual system

From `GUIConstants` and screen/component usage:

- Background: black
- Primary accent / active color: orange (`#FF9F0A`)
- Success: green
- Info: blue
- Warning: yellow
- Dire warning: orange-red
- Error: red
- Testnet and regtest have distinct amount colors

### Fonts

- Default UI text: **Open Sans**
- Fixed-width data display: **Inconsolata**
- Icon fonts:
  - **Font Awesome** for generic symbols (camera, dice, etc.)
  - **SeedSigner custom icon font** for app-specific icons
- Localized font substitutions exist for Arabic, Chinese, Hindi, Japanese, Korean, Persian, Thai.

### Resource assets

Bundled assets visibly used by the UI include:

- Fonts:
  - OpenSans-Regular / SemiBold
  - Inconsolata-Regular / SemiBold
  - NotoSans* locale-specific fonts
  - SeedSigner icon font
  - Font Awesome solid
- Bitmaps/images:
  - SeedSigner logo
  - BTC logo variants
  - warning / dire-warning bitmaps
  - partner/sponsor logo (`hrf_logo.png`)

### Recurrent data/display formats

These show up repeatedly and should be treated as first-class design primitives:

- **QR codes**
  - static QR
  - animated QR
  - adjustable brightness during display
  - full QR and zoomed-zone transcription modes
- **Bitcoin amounts**
  - btc / sats / threshold / hybrid display modes
  - testnet/regtest visual differentiation
- **Formatted Bitcoin addresses**
  - fixed-width rendering
  - first/last characters emphasized
  - line wrapping or truncation depending on context
- **Fingerprints**
  - short, prominent seed identity token
- **Derivation paths**
  - shown as fixed-ish technical strings
- **Lists and selectable menus**
  - compact button lists with scrolling
  - large home buttons with icon + label
- **Seed words**
  - numbered word list cards
  - candidate word list while typing
- **Passphrase widgets**
  - multi-layout keyboard with charset toggles
  - cursor movement, insert/delete, visible whitespace handling
- **Progress/status**
  - loading spinner/orbit animation
  - scan completion percentage + frame-status dot
  - address verification progress + skip feature
  - warning-edge pulsing on sensitive screens

## Reusable UI primitives and components

These are not full product screens; they are the design system pieces that recur across flows.

### Screen primitives

- `BaseTopNavScreen`
  - title + optional back/power buttons
- `ButtonListScreen`
  - vertically stacked buttons, centered or left-aligned, optional scrolling arrows
- `LargeButtonScreen`
  - 2 or 4 large icon buttons, used for home/power menus
- `KeyboardScreen`
  - text-entry display + keyboard grid + optional save button
- `QRDisplayScreen`
  - single/animated QR presentation with brightness controls
- `LargeIconStatusScreen`
  - icon + headline + body + bottom button
- Variants:
  - `WarningScreen`
  - `DireWarningScreen`
  - `ErrorScreen`

### Standalone components

- `TopNav`
- `Button`
- `CheckedSelectionButton`
- `CheckboxButton`
- `IconButton`
- `LargeIconButton`
- `TextArea`
- `ScrollableTextLine`
- `Icon`
- `IconTextLine`
- `FormattedAddress`
- `BtcAmount`
- `ToastOverlay` family
- `LoadingScreenThread` spinner
- `WarningEdgesMixin` / pulsing border effect

### Keyboard/input subsystem

- Grid keyboard with directional navigation and wrap behavior
- Additional soft keys:
  - backspace
  - space
  - cursor left/right
  - previous-page key support
- `TextEntryDisplay`
  - bar cursor mode
  - centered or left-aligned entry

### Reusable design patterns

- Top-nav escape hatch on almost every screen
- Bottom-anchored primary action list for confirm/review flows
- Technical-data pairing rows via icon + label + value
- Fixed-width rendering for addresses/xpubs/bits/passphrases
- Orange highlight to indicate active selection or sensitive checksum bits
- Danger/warning edge glow on seed-revealing or privacy-sensitive screens

## Functional inventory: full screens and flows

## 1) App shell, startup, and system screens

### Main shell

- **Main menu** (`MainMenuView` → `MainMenuScreen`)
  - Four large buttons:
    - Scan
    - Seeds
    - Tools
    - Settings
  - Uses SeedSigner section icons.
  - Includes power button in top nav.

- **Power options** (`PowerOptionsView` → `LargeButtonScreen`)
  - Restart
  - Power off

### Startup / attract / idle

- **Opening splash** (`OpeningSplashScreen`)
  - SeedSigner logo
  - version number
  - optional sponsor/partner credit block
  - fade-in behavior in non-screenshot mode

- **Screensaver** (`ScreensaverScreen`)
  - Bouncing logo on black background
  - restores previous screen on exit

### Generic system/status/error pages

- **Reset screen**
- **Power-off-not-required / “Just Unplug It”**
- **Work-in-progress placeholder**
- **Error / warning / dire warning pages**
- **Hardware error: cannot access camera**
- **Network mismatch**
- **Option disabled**
- **Remove MicroSD warning**

These largely reuse generic status/warning screens instead of dedicated bespoke layouts.

## 2) Scan domain

### Camera-backed screens

- **QR scan live preview** (`ScanScreen`) **[camera-backed]**
  - Full-screen camera preview
  - bottom instruction text initially
  - progress overlay once multipart scan begins
  - percentage readout
  - frame-status dot: success/repeat/miss feedback
  - currently central to all scan-driven flows

### Scan flows routed from the same scanner

The scanner is generic; post-scan routing determines destination:

- scan any QR
- scan PSBT
- scan SeedQR
- scan wallet descriptor
- scan address QR
- scan sign-message QR payload
- ingest settings QR

### Scan outcome/status screens

- **Unknown/invalid QR type** warning screen
- **Wrong QR type** error screen for constrained flows

### Important formats visible in this domain

- Static and animated QR scanning
- Multipart completion percentage
- Camera preview framing
- Explicit “back” affordance overlay

## 3) Seeds domain

This is the largest domain and the most important for LVGL parity.

### Seed landing / selection

- **In-memory seeds list** (`SeedsMenuView` → generic `ButtonListScreen`)
  - existing seeds shown by fingerprint
  - action to load a seed
- **Load a seed** (`LoadSeedView` → generic `ButtonListScreen`)
  - Scan SeedQR
  - Enter 12-word seed
  - Enter 24-word seed
  - Enter Electrum seed
  - Create a seed
- **Reusable seed selector** (`SeedSelectSeedView` → `SeedSelectSeedScreen`)
  - used by verify-address and sign-message flows
  - combines loaded seeds + load/scan/type paths

### Seed entry and validation

- **Seed word entry** (`SeedMnemonicEntryScreen`)
  - hybrid layout:
    - alphabet keyboard on left
    - candidate word list on right
  - current typed letters shown above keyboard
  - right-side highlighted candidate row with up/down controls
  - optimized around BIP-39 English word disambiguation
- **Invalid mnemonic** (`DireWarningScreen` via `SeedMnemonicInvalidView`)
  - checksum failure
  - review/edit vs discard
- **Electrum warning** generic warning page

### Seed finalization / seed identity

- **Finalize seed** (`SeedFinalizeScreen`)
  - fingerprint-centric confirmation
  - optional BIP-39 passphrase entry path
- **Seed options** (`SeedOptionsScreen`)
  - fingerprint in top nav
  - central seed action hub
  - view content depends on flow/state
- **Discard seed** warning flow

### Seed reveal / backup

- **View seed words warning** (`SeedWordsWarningView` via warning screen)
- **Seed words pages** (`SeedWordsScreen`)
  - numbered word chips/cards
  - paged display (`Seed Words: x/y`)
  - strong warning styling
- **Backup verify prompt** (`SeedWordsBackupTestPromptScreen`)
- **Backup test flow** (view-driven, uses generic screens for success/mistake states)
  - prompt
  - test interaction
  - mistake screen
  - success screen

### Passphrase flow

- **Passphrase entry** (`SeedAddPassphraseScreen`)
  - technically one of the richest custom screens in the app
  - text entry field at top
  - multi-layout keyboard sets:
    - lowercase
    - uppercase
    - digits
    - symbols set 1
    - symbols set 2
  - right-side hardware soft-button column:
    - toggle case
    - cycle digits/symbol pages
    - confirm/save
  - cursor movement, insert, delete, spaces
- **Passphrase exit dialog** (generic warning screen)
  - edit / discard / skip
- **Verify passphrase** (`SeedReviewPassphraseScreen`)
  - shows fingerprint change: before >> after
  - renders passphrase prominently in fixed-width orange text
  - replaces leading/trailing/double spaces with block glyphs for visibility

### Xpub export flow

A mix of generic list screens, one custom detail screen, and generic QR display.

- **Export Xpub: sig type** (generic list)
- **Export Xpub: script type** (generic list)
- **Custom derivation path entry** (`SeedExportXpubCustomDerivationScreen`)
- **Xpub QR format selection** (generic list)
- **Privacy leak warning** (generic warning)
- **Xpub details** (`SeedExportXpubDetailsScreen`)
  - fingerprint
  - derivation path
  - truncated xpub in fixed-width text
  - warning styling
- **Xpub QR display** (`QRDisplayScreen` via `SeedExportXpubQRDisplayView`)
  - format may be static xpub / UR / Specter legacy depending on settings

### SeedQR export / transcription flow

This area is especially relevant for design migration because it includes multiple QR-specific views.

- **SeedQR format choice** (`SeedTranscribeSeedQRFormatScreen`)
  - Standard = BIP-39 wordlist indices
  - Compact = raw entropy bits
- **SeedQR warning** (generic warning)
- **Whole SeedQR display** (`SeedTranscribeSeedQRWholeQRScreen`)
  - full QR on screen
  - button like `Begin 21x21`, `25x25`, `29x29`
  - warning styling
- **Zoomed SeedQR transcription helper** (`SeedTranscribeSeedQRZoomedInScreen`)
  - one zone at a time
  - center window with masked surroundings
  - gridlines over QR
  - zone labels like `A-3`, `D-5`
  - panning between zones with joystick
  - explicit click-to-exit instruction
- **Confirm transcribed QR prompt** (`SeedTranscribeSeedQRConfirmQRPromptScreen`)
- **Confirm by rescanning** (`SeedTranscribeSeedQRConfirmScanView` → `ScanScreen`) **[camera-adjacent / camera-backed]**
- **Wrong seed / invalid QR / success** status screens via generic warning/success patterns

### BIP85 / derived seed flow

- **Select number of words** (generic list)
- **Select child index** (`SeedBIP85SelectChildIndexScreen`)
  - numeric keyboard + save
- **Invalid child index** warning
- resulting seed likely routes back into standard seed reveal/finalize surfaces

### Address verification flow

This domain straddles seeds and tools, but the custom verification screen lives in seed screens.

- **Address verification start** (flow entry from scanned address)
- **Signature type selection** (`AddressVerificationSigTypeScreen`)
- **Seed selector** (`SeedSelectSeedScreen`)
- **Address verification progress screen** (`SeedAddressVerificationScreen`)
  - formatted address at top
  - sig/script/network line below
  - threaded brute-force progress/search process
  - includes “Skip 10” style progression behavior at flow layer
- **Address verification success** (`SeedAddressVerificationSuccessScreen`)
- **Address verification failed** warning screen in PSBT path

### Multisig descriptor flow

- **Load multisig wallet descriptor** (`LoadMultisigWalletDescriptorScreen`)
- **Multisig wallet descriptor summary** (`MultisigWalletDescriptorScreen`)
  - descriptor-centric review before later actions
  - exact content is flow-driven, but this is a distinct dedicated surface

### Sign message flow

- **Sign message start** (view-level setup)
- **Confirm message** (`SeedSignMessageConfirmMessageScreen`)
- **Confirm address** (`SeedSignMessageConfirmAddressScreen`)
- **Signed message QR display** (`QRDisplayScreen` via `SeedSignMessageSignedMessageQRView`)

## 4) PSBT / transaction review domain

This is the second major complex domain after Seeds.

### PSBT entry and signer choice

- **Select signer** (`PSBTSelectSeedView` → generic `ButtonListScreen`)
  - loaded seed fingerprints
  - scan seed
  - type 12/24 word seed
  - optional Electrum

### Review and explain transaction

- **Transaction overview** (`PSBTOverviewScreen`)
  - title: Review Transaction
  - prominent spend amount (`BtcAmount`)
  - diagrammatic flow from inputs to recipients/change/fee/OP_RETURN
  - adapts between self-transfer, many inputs, many outputs
- **Unsupported script type warning** (generic warning)
- **Full spend / no change warning** (generic warning)
- **Transaction math** (`PSBTMathScreen`)
  - equation-like vertical breakdown:
    - inputs
    - recipients
    - fee
    - resulting change
  - fixed-width amount alignment
  - uses secondary/tertiary digit coloring in btc mode

### Recipient / output inspection

- **Recipient address details** (`PSBTAddressDetailsScreen`)
  - amount + formatted address
- **Change details** (`PSBTChangeDetailsScreen`)
  - amount
  - change/receive address label + index number
  - formatted address
  - optional “Address verified!” success row
- **OP_RETURN details** (`PSBTOpReturnScreen`)
  - if UTF-8 text: show as human-readable text
  - otherwise: show raw hex in fixed-width layout

### Signing

- **Finalize / approve transaction** (`PSBTFinalizeScreen`)
  - large sign icon
  - “Click to approve this transaction”
- **Signed PSBT QR display** (`QRDisplayScreen`) 
- **Signing error** generic error/warning screen

## 5) Tools domain

### Tools menu

- **Tools landing menu** (generic `ButtonListScreen`)
  - New seed (camera entropy)
  - New seed (dice)
  - Calc 12th/24th word
  - Address explorer
  - Verify address

### Image entropy flow

- **Live camera preview for entropy capture** (`ToolsImageEntropyLivePreviewScreen`) **[camera-backed]**
  - edge-to-edge preview
  - instruction overlay: back / click a button
  - captures rolling preview frames for extra entropy
- **Final captured image review** (`ToolsImageEntropyFinalImageScreen`) **[camera-adjacent]**
  - accept vs reshoot overlay
- **Mnemonic length selection** (generic list)
- result routes into seed warning/reveal flow

### Dice entropy flow

- **Mnemonic length selection** (generic list with roll counts)
- **Dice roll entry** (`ToolsDiceEntropyEntryScreen`)
  - 3x3 icon keyboard
  - dice face icons map to digits 1–6
  - title updates as `Dice Roll x/y`
- result routes into seed warning/reveal flow

### Final-word calculator flow

- **Select 12 vs 24 words** (generic list)
- **Finalize prompt** (`ToolsCalcFinalWordFinalizePromptScreen`)
  - explains entropy bits + checksum relationship
- **Coin flip entry** (`ToolsCoinFlipEntryScreen`)
  - `1` / `0` keyboard
  - explanatory text: Heads = 1, Tails = 0
- **Manual penultimate-word entry** uses seed mnemonic entry screen pattern
- **Final-word calculation display** (`ToolsCalcFinalWordScreen`)
  - user input bits
  - discarded bits vs checksum bits
  - checksum highlighted in orange
  - resulting final word
  - fixed-width bit visualization
- **Final word done** (`ToolsCalcFinalWordDoneScreen`)
  - big quoted final word
  - seed fingerprint beneath

### Address explorer flow

This uses both tool-specific screens and generic QR display.

- **Select source** (seed vs descriptor flow; view-driven generic list)
- **Address type / derivation summary** (`ToolsAddressExplorerAddressTypeScreen`)
  - fingerprint or descriptor name
  - derivation display
- **Address list** (`ToolsAddressExplorerAddressListScreen`)
  - fixed-width truncated addresses with index prefix
  - selected item expands to full address as active label
  - “Next N” pagination button
- **Single address view / export** (`ToolsAddressExplorerAddressView` flow)
  - uses generic QR display for address QR
  - may reuse formatted-address presentation concepts even though not a dedicated custom screen here

### Verify address shortcut

- Tools menu routes into scanned-address verification flow; screen surface is shared with Seeds domain.

## 6) Settings domain

### Settings navigation

- **Settings menu** (generic `ButtonListScreen`)
  - dynamically generated from settings definitions
  - nested sections:
    - Settings
    - Advanced
    - Hardware
    - Developer options (visibility-gated)
  - static extras:
    - I/O test
    - Donate

### Selection/update pages

- **Settings entry update selection** (`SettingsEntryUpdateSelectionScreen`)
  - title + display name + optional help text
  - uses checkmark or checkbox variants depending on setting type
  - supports multiselect and single select
- **Locale selection** reuses the same screen with per-language font overrides
- **Selection required warning** generic warning flow

### Settings QR flow

- **Ingest settings QR** (`SettingsIngestSettingsQRView` → `ScanScreen`) **[camera-adjacent / camera-backed]**
- **Settings QR confirmation** (`SettingsQRConfirmationScreen`)
  - optional config name in quotes
  - status message like “Settings updated...”
  - single Home action

### Hardware test / support

- **I/O test** (`IOTestScreen`) **[camera-adjacent / camera-backed]**
  - D-pad pictogram showing joystick directions + center click
  - right-side hardware key column:
    - camera key
    - clear key
    - exit key
  - can capture and show a camera image in the background
- **Donate** (`DonateScreen`)
  - explanatory text
  - highlighted `seedsigner.com`

## Camera-backed and camera-adjacent inventory

### Directly camera-backed

- `ScanScreen`
- `ToolsImageEntropyLivePreviewScreen`
- `IOTestScreen` (camera capture mode)
- any flow reusing `ScanScreen`:
  - scan any QR
  - scan PSBT
  - scan SeedQR
  - scan descriptor
  - scan address
  - ingest settings QR
  - confirm transcribed SeedQR by rescanning

### Camera-adjacent but not live-preview-first

- `ToolsImageEntropyFinalImageScreen`
- camera-connection error flow
- SeedQR transcription confirmation flows, because they depend on a follow-up scan
- many scan-routed business flows, although their dedicated screens are not themselves camera surfaces

## Most reusable patterns for an LVGL port

If the goal is a design-faithful but maintainable port, these appear to be the highest-value reusable patterns:

1. **Top nav + hardware-button focus model**
2. **Standard list screen** with scroll arrows and long-label handling
3. **Large home button grid**
4. **Status/warning screen family**
5. **QR display surface** with animated/static support and brightness control
6. **QR scan surface** with preview, progress, and status dot
7. **Keyboard framework** with alternate layouts and side soft-buttons
8. **Formatted technical data rows**:
   - fingerprint
   - derivation
   - xpub
   - address
   - bitcoin amount
9. **Sensitive reveal styling**:
   - dire warning edges
   - fixed-width data presentation
   - orange checksum emphasis

## Areas where the design is especially SeedSigner-specific

These should not be flattened into generic dialogs during migration:

- Seed mnemonic entry with candidate-word panel
- Passphrase entry with charset toggles and cursor editing
- PSBT overview flowchart and PSBT math equation layout
- SeedQR transcription pair:
  - full QR overview
  - zoomed zone navigator
- Address verification progress/search UI
- Final-word calculator bit visualization

## Historical grounding / stability notes

A few surfaces are clearly stable enough to treat as long-lived product UI, not incidental implementation:

- Main menu structure: Scan / Seeds / Tools / Settings
- Generic warning/error/status pages
- Scan → route-by-QR-type model
- Seed loading via QR or manual mnemonic entry
- Passphrase verification flow
- PSBT review → details → sign pipeline
- Tools for entropy generation and final-word calculation
- Settings QR ingestion and hardware I/O test

Some code comments indicate internal refactoring pressure, but not product-surface instability:

- early splash/screensaver code still calls out old Screen/View boundaries
- scan/live-preview code notes performance-driven compromises on Pi Zero
- several warnings mention future refactors, but the screen concepts themselves remain useful and coherent

## Suggested decomposition for the LVGL project backlog

A practical design decomposition from this inventory would be:

- **Foundation primitives**
  - nav bar
  - button/list system
  - icon/text rows
  - keyboard/text entry
  - QR display + QR scan containers
- **Shell/system**
  - splash
  - screensaver
  - main menu
  - warning/error/success family
- **Seed core**
  - seed entry
  - seed reveal/backup
  - passphrase
- **PSBT core**
  - overview
  - math
  - output details
  - sign/export QR
- **Tools core**
  - image entropy
  - dice
  - final word
  - address explorer
- **Settings core**
  - dynamic settings list
  - selection screens
  - settings QR + I/O test

## Source map appendix

### Dedicated screen classes

- `gui/screens/screen.py`
  - `MainMenuScreen`
  - `QRDisplayScreen`
  - `KeyboardScreen`
  - `WarningScreen` / `DireWarningScreen` / `ErrorScreen`
  - `ResetScreen`
  - `PowerOffNotRequiredScreen`
- `gui/screens/scan_screens.py`
  - `ScanScreen`
- `gui/screens/seed_screens.py`
  - `SeedMnemonicEntryScreen`
  - `SeedFinalizeScreen`
  - `SeedOptionsScreen`
  - `SeedWordsScreen`
  - `SeedBIP85SelectChildIndexScreen`
  - `SeedWordsBackupTestPromptScreen`
  - `SeedExportXpubCustomDerivationScreen`
  - `SeedExportXpubDetailsScreen`
  - `SeedAddPassphraseScreen`
  - `SeedReviewPassphraseScreen`
  - `SeedTranscribeSeedQRFormatScreen`
  - `SeedTranscribeSeedQRWholeQRScreen`
  - `SeedTranscribeSeedQRZoomedInScreen`
  - `SeedTranscribeSeedQRConfirmQRPromptScreen`
  - `AddressVerificationSigTypeScreen`
  - `SeedSelectSeedScreen`
  - `SeedAddressVerificationScreen`
  - `SeedAddressVerificationSuccessScreen`
  - `LoadMultisigWalletDescriptorScreen`
  - `MultisigWalletDescriptorScreen`
  - `SeedSignMessageConfirmMessageScreen`
  - `SeedSignMessageConfirmAddressScreen`
- `gui/screens/psbt_screens.py`
  - `PSBTOverviewScreen`
  - `PSBTMathScreen`
  - `PSBTAddressDetailsScreen`
  - `PSBTChangeDetailsScreen`
  - `PSBTOpReturnScreen`
  - `PSBTFinalizeScreen`
- `gui/screens/tools_screens.py`
  - `ToolsImageEntropyLivePreviewScreen`
  - `ToolsImageEntropyFinalImageScreen`
  - `ToolsDiceEntropyEntryScreen`
  - `ToolsCalcFinalWordFinalizePromptScreen`
  - `ToolsCoinFlipEntryScreen`
  - `ToolsCalcFinalWordScreen`
  - `ToolsCalcFinalWordDoneScreen`
  - `ToolsAddressExplorerAddressTypeScreen`
  - `ToolsAddressExplorerAddressListScreen`
- `gui/screens/settings_screens.py`
  - `SettingsEntryUpdateSelectionScreen`
  - `IOTestScreen`
  - `DonateScreen`
  - `SettingsQRConfirmationScreen`
- `views/screensaver.py`
  - `OpeningSplashScreen`
  - `ScreensaverScreen`

### Important view-only flow nodes using generic screens

- `ScanView`, `ScanPSBTView`, `ScanSeedQRView`, `ScanWalletDescriptorView`, `ScanAddressView`
- `SeedsMenuView`, `LoadSeedView`, `SeedMnemonicInvalidView`, `SeedAddPassphraseExitDialogView`, `SeedDiscardView`, `SeedWordsWarningView`, `SeedExportXpubSigTypeView`, `SeedExportXpubScriptTypeView`, `SeedExportXpubQRFormatView`, `SeedExportXpubWarningView`, `SeedExportXpubQRDisplayView`, `SeedTranscribeSeedQRWarningView`, `SeedTranscribeSeedQRConfirmScanView`, `SeedTranscribeSeedQRConfirmWrongSeedView`, `SeedTranscribeSeedQRConfirmInvalidQRView`, `SeedTranscribeSeedQRConfirmSuccessView`, `AddressVerificationStartView`, `LoadMultisigWalletDescriptorView`, `SeedSignMessageSignedMessageQRView`
- `PSBTSelectSeedView`, `PSBTUnsupportedScriptTypeWarningView`, `PSBTNoChangeWarningView`, `PSBTSignedQRDisplayView`, `PSBTSigningErrorView`
- `ToolsMenuView`, `ToolsImageEntropyMnemonicLengthView`, `ToolsDiceEntropyMnemonicLengthView`, `ToolsCalcFinalWordNumWordsView`, `ToolsAddressExplorerSelectSourceView`, `ToolsAddressExplorerAddressView`
- `SettingsMenuView`, `LocaleSelectionView`, `SettingsIngestSettingsQRView`, `SettingsSelectionRequiredWarningView`
