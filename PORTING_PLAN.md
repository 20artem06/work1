# Qt Porting Plan

This document tracks the staged migration of the legacy MFC application to a Qt Widgets based project. The goal is to keep every step reviewable and runnable inside Qt Creator 5 while progressively recreating the original behaviour.

## Stage 1 — Qt project bootstrap *(completed)*
- Replace the MFC-oriented build configuration with a Qt-enabled CMake project.
- Introduce a stub `QMainWindow` based application to validate the toolchain, auto-generated code (MOC/UIC) and project layout.
- Provide a visible placeholder window signalling that the Qt port is in progress.

## Stage 2 — Data model migration *(current)*
- Move the core domain classes from `Video.h/.cpp` and `Session.h/.cpp` to Qt/standard types (`QString`, `QVector`, etc.).
- Preserve existing parsing, serialization and calculation logic while decoupling from MFC specific constructs.
- Add unit tests (QTest or Qt based) that cover grid loading, session lifecycle and persistence routines. *(pending)*

## Stage 3 — Main window & project management
- Recreate the main window menus, toolbars and project handling workflow from `MainFrm.cpp` using Qt actions, menus and dock widgets.
- Port the helper dialogs (`CGraphSel`, `CDelProjectDlg`, etc.) to dedicated `QDialog` classes or `.ui` forms with equivalent behaviour.
- Implement project persistence hooks connected to the migrated data model.

## Stage 4 — Graph visualisation workspaces
- Re-implement `CChild_1_Frame` and `CChild_2_Frame` as Qt widgets (likely leveraging `QMdiArea` + custom `QWidget`/`QGraphicsView`).
- Reproduce navigation, drawing and export capabilities, coordinating with the data model.
- Support image export via Qt's painting stack (`QImage`, `QPrinter`).

## Stage 5 — Specialised dialogs and tools
- Port AVI export, palette configuration and other specialised dialogs (`AviDlg`, `ColorsDlg`, `F_1_dlg`, `F_2_dlg`, etc.) to Qt.
- Replace message maps with signals/slots and ensure long-running operations provide UI feedback.
- Integrate palette and AVI helpers with Qt equivalents (`QImageWriter`, `QFileDialog`).

## Stage 6 — Resource migration & polish
- Convert icons, accelerators and localisation resources from `resource.h`/`video.rc` into Qt resource files and translation catalogs.
- Ensure keyboard shortcuts and accessibility mirrors the original MFC application.

## Stage 7 — Regression testing & release packaging
- Build automated regression scenarios covering project creation/editing, chart manipulation, AVI export and palette adjustments.
- Finalise installers or deployment scripts suitable for Qt distributions and produce release documentation.

Each stage will result in an incremental commit and review, gradually replacing the legacy MFC implementation with a native Qt solution.
