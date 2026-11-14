# GUI & 2D/3D Drawing (DX & CEGUI)

2D screen drawing, fonts, text, rectangles, lines, textures, render targets, GUI windows, buttons, edits, labels, gridlists, and 3D line drawing.

## Parity Status

- **Total Functions:** 199
- **Implemented:** 136 (68.3%)
- **Stubs:** 5
- **Missing:** 58

> **Technical Requirement:** Desktop uses CEGUI and Direct3D9. Mobile uses `MtaDxView.java` (Android Canvas) + `LuaGuiApi.cpp`. Windows, edits, memos, tabs, scroll panes and grid lists are stored and drawn into the DX frame. Grid sorting does not reorder rows. `guiCreateFont` returns false because CreateGuiFont is not hooked. Shaders (HLSL -> GLSL ES) and the browser are still missing.

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `addModel2DFX` | Missing | `CLua2DFXDefs.cpp:568` | `AddModel2DFX` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `dxConvertPixels` | Missing | `CLuaDrawingDefs.cpp:60` | `DxConvertPixels` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxCreateFont` | Missing | `CLuaDrawingDefs.cpp:43` | `DxCreateFont` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxCreateRenderTarget` | Missing | `CLuaDrawingDefs.cpp:46` | `DxCreateRenderTarget` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxCreateScreenSource` | Missing | `CLuaDrawingDefs.cpp:47` | `DxCreateScreenSource` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxCreateShader` | Missing | `CLuaDrawingDefs.cpp:45` | `DxCreateShader` | Needs GLSL ES 3.0 shader pipeline (translating desktop D3DX9 HLSL). |
| `dxCreateTexture` | Missing | `CLuaDrawingDefs.cpp:44` | `DxCreateTexture` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawCircle` | *Stub* | `CLuaDrawingDefs.cpp:31` | `DxDrawCircle` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `dxDrawImage` | **Implemented** | `CLuaDrawingDefs.cpp:32` | `DxDrawImage` | Operational on Android. (l_dxDrawImage) |
| `dxDrawImageSection` | **Implemented** | `CLuaDrawingDefs.cpp:33` | `DxDrawImageSection` | Operational on Android. (l_dxDrawImageSection) |
| `dxDrawLine` | Missing | `CLuaDrawingDefs.cpp:25` | `DxDrawLine` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawLine3D` | Missing | `CLuaDrawingDefs.cpp:28` | `DxDrawLine3D` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawMaterialLine3D` | Missing | `CLuaDrawingDefs.cpp:26` | `DxDrawMaterialLine3D` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawMaterialPrimitive` | Missing | `CLuaDrawingDefs.cpp:36` | `DxDrawMaterialPrimitive` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawMaterialPrimitive3D` | Missing | `CLuaDrawingDefs.cpp:37` | `DxDrawMaterialPrimitive3D` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawMaterialSectionLine3D` | Missing | `CLuaDrawingDefs.cpp:27` | `DxDrawMaterialSectionLine3D` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawModel3D` | Missing | `CLuaDrawingDefs.cpp:39` | `ArgumentParser<DxDrawModel3D>` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawPrimitive` | *Stub* | `CLuaDrawingDefs.cpp:34` | `DxDrawPrimitive` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `dxDrawPrimitive3D` | Missing | `CLuaDrawingDefs.cpp:35` | `DxDrawPrimitive3D` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxDrawRectangle` | **Implemented** | `CLuaDrawingDefs.cpp:30` | `DxDrawRectangle` | Operational on Android. (l_dxDrawRectangle) |
| `dxDrawText` | **Implemented** | `CLuaDrawingDefs.cpp:29` | `DxDrawText` | Operational on Android. (l_dxDrawText) |
| `dxDrawWiredSphere` | Missing | `CLuaDrawingDefs.cpp:38` | `ArgumentParser<DxDrawWiredSphere>` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxGetBlendMode` | Missing | `CLuaDrawingDefs.cpp:64` | `DxGetBlendMode` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxGetFontHeight` | **Implemented** | `CLuaDrawingDefs.cpp:42` | `DxGetFontHeight` | Operational on Android. (DxGetFontHeight) |
| `dxGetMaterialSize` | Missing | `CLuaDrawingDefs.cpp:48` | `DxGetMaterialSize` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxGetPixelColor` | **Implemented** | `CLuaDrawingDefs.cpp:61` | `DxGetPixelColor` | Operational on Android. (DxGetPixelColor) |
| `dxGetPixelsFormat` | **Implemented** | `CLuaDrawingDefs.cpp:59` | `DxGetPixelsFormat` | Operational on Android. (DxGetPixelsFormat) |
| `dxGetPixelsSize` | Missing | `CLuaDrawingDefs.cpp:58` | `DxGetPixelsSize` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxGetStatus` | Missing | `CLuaDrawingDefs.cpp:54` | `DxGetStatus` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxGetTextSize` | Missing | `CLuaDrawingDefs.cpp:41` | `ArgumentParser<DxGetTextSize>` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxGetTextWidth` | **Implemented** | `CLuaDrawingDefs.cpp:40` | `DxGetTextWidth` | Operational on Android. (DxGetTextWidth) |
| `dxGetTexturePixels` | Missing | `CLuaDrawingDefs.cpp:56` | `DxGetTexturePixels` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxIsAspectRatioAdjustmentEnabled` | Missing | `CLuaDrawingDefs.cpp:66` | `DxIsAspectRatioAdjustmentEnabled` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxSetAspectRatioAdjustmentEnabled` | Missing | `CLuaDrawingDefs.cpp:65` | `DxSetAspectRatioAdjustmentEnabled` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxSetBlendMode` | Missing | `CLuaDrawingDefs.cpp:63` | `DxSetBlendMode` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxSetPixelColor` | **Implemented** | `CLuaDrawingDefs.cpp:62` | `DxSetPixelColor` | Operational on Android. (DxSetPixelColor) |
| `dxSetRenderTarget` | Missing | `CLuaDrawingDefs.cpp:52` | `DxSetRenderTarget` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxSetShaderTessellation` | Missing | `CLuaDrawingDefs.cpp:50` | `DxSetShaderTessellation` | Needs GLSL ES 3.0 shader pipeline (translating desktop D3DX9 HLSL). |
| `dxSetShaderTransform` | Missing | `CLuaDrawingDefs.cpp:51` | `DxSetShaderTransform` | Needs GLSL ES 3.0 shader pipeline (translating desktop D3DX9 HLSL). |
| `dxSetShaderValue` | Missing | `CLuaDrawingDefs.cpp:49` | `DxSetShaderValue` | Needs GLSL ES 3.0 shader pipeline (translating desktop D3DX9 HLSL). |
| `dxSetTestMode` | Missing | `CLuaDrawingDefs.cpp:55` | `DxSetTestMode` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxSetTextureEdge` | Missing | `CLuaDrawingDefs.cpp:67` | `DxSetTextureEdge` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxSetTexturePixels` | Missing | `CLuaDrawingDefs.cpp:57` | `DxSetTexturePixels` | Needs Android Canvas / OpenGL ES draw line routing. |
| `dxUpdateScreenSource` | Missing | `CLuaDrawingDefs.cpp:53` | `DxUpdateScreenSource` | Needs Android Canvas / OpenGL ES draw line routing. |
| `getChatboxCharacterLimit` | Missing | `CLuaGUIDefs.cpp:54` | `ArgumentParser<GUIGetChatboxCharacterLimit>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getChatboxLayout` | Missing | `CLuaGUIDefs.cpp:186` | `GUIGetChatboxLayout` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getModel2DFXCount` | Missing | `CLua2DFXDefs.cpp:573` | `GetModel2DFXCount` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getModel2DFXEffects` | Missing | `CLua2DFXDefs.cpp:572` | `GetModel2DFXEffects` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getModel2DFXPosition` | Missing | `CLua2DFXDefs.cpp:571` | `GetModel2DFXPosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getModel2DFXProperty` | Missing | `CLua2DFXDefs.cpp:572` | `GetModel2DFXProperty` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getModel2DFXType` | Missing | `CLua2DFXDefs.cpp:573` | `GetModel2DFXType` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `guiBlur` | **Implemented** | `CLuaGUIDefs.cpp:132` | `GUIBlur` | Operational on Android. (GuiBlur) |
| `guiBringToFront` | **Implemented** | `CLuaGUIDefs.cpp:130` | `GUIBringToFront` | Operational on Android. (GuiBringToFront) |
| `guiCheckBoxGetSelected` | **Implemented** | `CLuaGUIDefs.cpp:149` | `GUICheckBoxGetSelected` | Operational on Android. (GuiCheckBoxGetSelected) |
| `guiCheckBoxSetSelected` | **Implemented** | `CLuaGUIDefs.cpp:135` | `GUICheckBoxSetSelected` | Operational on Android. (GuiCheckBoxSetSelected) |
| `guiComboBoxAddItem` | **Implemented** | `CLuaGUIDefs.cpp:188` | `GUIComboBoxAddItem` | Operational on Android. (GuiComboBoxAddItem) |
| `guiComboBoxClear` | **Implemented** | `CLuaGUIDefs.cpp:190` | `GUIComboBoxClear` | Operational on Android. (GuiComboBoxClear) |
| `guiComboBoxGetItemCount` | **Implemented** | `CLuaGUIDefs.cpp:195` | `GUIComboBoxGetItemCount` | Operational on Android. (GuiComboBoxGetItemCount) |
| `guiComboBoxGetItemText` | **Implemented** | `CLuaGUIDefs.cpp:193` | `GUIComboBoxGetItemText` | Operational on Android. (GuiComboBoxGetItemText) |
| `guiComboBoxGetSelected` | **Implemented** | `CLuaGUIDefs.cpp:191` | `GUIComboBoxGetSelected` | Operational on Android. (GuiComboBoxGetSelected) |
| `guiComboBoxIsOpen` | **Implemented** | `CLuaGUIDefs.cpp:197` | `GUIComboBoxIsOpen` | Operational on Android. (GuiComboBoxIsOpen) |
| `guiComboBoxRemoveItem` | **Implemented** | `CLuaGUIDefs.cpp:189` | `GUIComboBoxRemoveItem` | Operational on Android. (GuiComboBoxRemoveItem) |
| `guiComboBoxSetItemText` | **Implemented** | `CLuaGUIDefs.cpp:194` | `GUIComboBoxSetItemText` | Operational on Android. (GuiComboBoxSetItemText) |
| `guiComboBoxSetOpen` | **Implemented** | `CLuaGUIDefs.cpp:196` | `GUIComboBoxSetOpen` | Operational on Android. (GuiComboBoxSetOpen) |
| `guiComboBoxSetSelected` | **Implemented** | `CLuaGUIDefs.cpp:192` | `GUIComboBoxSetSelected` | Operational on Android. (GuiComboBoxSetSelected) |
| `guiCreateButton` | **Implemented** | `CLuaGUIDefs.cpp:58` | `GUICreateButton` | Operational on Android. (GuiCreateButton) |
| `guiCreateCheckBox` | **Implemented** | `CLuaGUIDefs.cpp:67` | `GUICreateCheckBox` | Operational on Android. (GuiCreateCheckBox) |
| `guiCreateComboBox` | **Implemented** | `CLuaGUIDefs.cpp:70` | `GUICreateComboBox` | Operational on Android. (GuiCreateComboBox) |
| `guiCreateEdit` | **Implemented** | `CLuaGUIDefs.cpp:59` | `GUICreateEdit` | Operational on Android. (GuiCreateEdit) |
| `guiCreateFont` | **Implemented** | `CLuaGUIDefs.cpp:71` | `GUICreateFont` | Operational on Android. (GuiCreateFont) |
| `guiCreateGridList` | **Implemented** | `CLuaGUIDefs.cpp:61` | `GUICreateGridList` | Operational on Android. (GuiCreateGridList) |
| `guiCreateLabel` | **Implemented** | `CLuaGUIDefs.cpp:57` | `GUICreateLabel` | Operational on Android. (GuiCreateLabel) |
| `guiCreateMemo` | **Implemented** | `CLuaGUIDefs.cpp:60` | `GUICreateMemo` | Operational on Android. (GuiCreateMemo) |
| `guiCreateProgressBar` | **Implemented** | `CLuaGUIDefs.cpp:66` | `GUICreateProgressBar` | Operational on Android. (GuiCreateProgressBar) |
| `guiCreateRadioButton` | **Implemented** | `CLuaGUIDefs.cpp:68` | `GUICreateRadioButton` | Operational on Android. (GuiCreateRadioButton) |
| `guiCreateScrollBar` | **Implemented** | `CLuaGUIDefs.cpp:63` | `GUICreateScrollBar` | Operational on Android. (GuiCreateScrollBar) |
| `guiCreateScrollPane` | **Implemented** | `CLuaGUIDefs.cpp:62` | `GUICreateScrollPane` | Operational on Android. (GuiCreateScrollPane) |
| `guiCreateStaticImage` | **Implemented** | `CLuaGUIDefs.cpp:69` | `GUICreateStaticImage` | Operational on Android. (GuiCreateStaticImage) |
| `guiCreateTab` | **Implemented** | `CLuaGUIDefs.cpp:65` | `GUICreateTab` | Operational on Android. (GuiCreateTab) |
| `guiCreateTabPanel` | **Implemented** | `CLuaGUIDefs.cpp:64` | `GUICreateTabPanel` | Operational on Android. (GuiCreateTabPanel) |
| `guiCreateWindow` | **Implemented** | `CLuaGUIDefs.cpp:56` | `GUICreateWindow` | Operational on Android. (GuiCreateWindow) |
| `guiDeleteTab` | **Implemented** | `CLuaGUIDefs.cpp:77` | `GUIDeleteTab` | Operational on Android. (GuiDeleteTab) |
| `guiEditGetCaretIndex` | **Implemented** | `CLuaGUIDefs.cpp:158` | `GUIEditGetCaretIndex` | Operational on Android. (GuiEditGetCaretIndex) |
| `guiEditGetMaxLength` | **Implemented** | `CLuaGUIDefs.cpp:162` | `GUIEditGetMaxLength` | Operational on Android. (GuiEditGetMaxLength) |
| `guiEditIsMasked` | **Implemented** | `CLuaGUIDefs.cpp:160` | `GUIEditIsMasked` | Operational on Android. (GuiEditIsMasked) |
| `guiEditIsReadOnly` | **Implemented** | `CLuaGUIDefs.cpp:164` | `GUIEditIsReadOnly` | Operational on Android. (GuiEditIsReadOnly) |
| `guiEditSetCaretIndex` | **Implemented** | `CLuaGUIDefs.cpp:157` | `GUIEditSetCaretIndex` | Operational on Android. (GuiEditSetCaretIndex) |
| `guiEditSetMasked` | **Implemented** | `CLuaGUIDefs.cpp:159` | `GUIEditSetMasked` | Operational on Android. (GuiEditSetMasked) |
| `guiEditSetMaxLength` | **Implemented** | `CLuaGUIDefs.cpp:161` | `GUIEditSetMaxLength` | Operational on Android. (GuiEditSetMaxLength) |
| `guiEditSetReadOnly` | **Implemented** | `CLuaGUIDefs.cpp:163` | `GUIEditSetReadOnly` | Operational on Android. (GuiEditSetReadOnly) |
| `guiFocus` | **Implemented** | `CLuaGUIDefs.cpp:133` | `GUIFocus` | Operational on Android. (GuiFocus) |
| `guiGetAlpha` | **Implemented** | `CLuaGUIDefs.cpp:141` | `GUIGetAlpha` | Operational on Android. (GuiGetAlpha) |
| `guiGetCursorType` | **Implemented** | `CLuaGUIDefs.cpp:147` | `GUIGetCursorType` | Operational on Android. (GuiGetCursorType) |
| `guiGetEnabled` | **Implemented** | `CLuaGUIDefs.cpp:138` | `GUIGetEnabled` | Operational on Android. (GuiGetEnabled) |
| `guiGetFont` | **Implemented** | `CLuaGUIDefs.cpp:143` | `GUIGetFont` | Operational on Android. (GuiGetFont) |
| `guiGetInputEnabled` | **Implemented** | `CLuaGUIDefs.cpp:40` | `GUIGetInputEnabled` | Operational on Android. (GuiGetInputEnabled) |
| `guiGetInputMode` | **Implemented** | `CLuaGUIDefs.cpp:42` | `GUIGetInputMode` | Operational on Android. (GuiGetInputMode) |
| `guiGetPosition` | **Implemented** | `CLuaGUIDefs.cpp:145` | `GUIGetPosition` | Operational on Android. (GuiGetPosition) |
| `guiGetProperties` | **Implemented** | `CLuaGUIDefs.cpp:140` | `GUIGetProperties` | Operational on Android. (GuiGetProperties) |
| `guiGetProperty` | **Implemented** | `CLuaGUIDefs.cpp:139` | `GUIGetProperty` | Operational on Android. (GuiGetProperty) |
| `guiGetScreenSize` | **Implemented** | `CLuaGUIDefs.cpp:155` | `GUIGetScreenSize` | Operational on Android. (l_guiGetScreenSize) |
| `guiGetSelectedTab` | **Implemented** | `CLuaGUIDefs.cpp:75` | `GUIGetSelectedTab` | Operational on Android. (GuiGetSelectedTab) |
| `guiGetSize` | **Implemented** | `CLuaGUIDefs.cpp:144` | `GUIGetSize` | Operational on Android. (GuiGetSize) |
| `guiGetText` | **Implemented** | `CLuaGUIDefs.cpp:142` | `GUIGetText` | Operational on Android. (GuiGetText) |
| `guiGetVisible` | **Implemented** | `CLuaGUIDefs.cpp:146` | `GUIGetVisible` | Operational on Android. (GuiGetVisible) |
| `guiGridListAddColumn` | **Implemented** | `CLuaGUIDefs.cpp:81` | `GUIGridListAddColumn` | Operational on Android. (GuiGridListAddColumn) |
| `guiGridListAddRow` | **Implemented** | `CLuaGUIDefs.cpp:90` | `GUIGridListAddRow` | Operational on Android. (GuiGridListAddRow) |
| `guiGridListAutoSizeColumn` | **Implemented** | `CLuaGUIDefs.cpp:93` | `GUIGridListAutoSizeColumn` | Operational on Android. (GuiGridListAutoSizeColumn) |
| `guiGridListClear` | **Implemented** | `CLuaGUIDefs.cpp:94` | `GUIGridListClear` | Operational on Android. (GuiGridListClear) |
| `guiGridListGetColumnCount` | **Implemented** | `CLuaGUIDefs.cpp:89` | `GUIGridListGetColumnCount` | Operational on Android. (GuiGridListGetColumnCount) |
| `guiGridListGetColumnTitle` | **Implemented** | `CLuaGUIDefs.cpp:86` | `GUIGridListGetColumnTitle` | Operational on Android. (GuiGridListGetColumnTitle) |
| `guiGridListGetColumnWidth` | **Implemented** | `CLuaGUIDefs.cpp:84` | `GUIGridListGetColumnWidth` | Operational on Android. (GuiGridListGetColumnWidth) |
| `guiGridListGetHorizontalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:108` | `GUIGridListGetHorizontalScrollPosition` | Operational on Android. (GuiGridListGetHorizontalScrollPosition) |
| `guiGridListGetItemColor` | **Implemented** | `CLuaGUIDefs.cpp:100` | `GUIGridListGetItemColor` | Operational on Android. (GuiGridListGetItemColor) |
| `guiGridListGetItemData` | **Implemented** | `CLuaGUIDefs.cpp:98` | `GUIGridListGetItemData` | Operational on Android. (GuiGridListGetItemData) |
| `guiGridListGetItemText` | **Implemented** | `CLuaGUIDefs.cpp:96` | `GUIGridListGetItemText` | Operational on Android. (GuiGridListGetItemText) |
| `guiGridListGetRowCount` | **Implemented** | `CLuaGUIDefs.cpp:88` | `GUIGridListGetRowCount` | Operational on Android. (GuiGridListGetRowCount) |
| `guiGridListGetSelectedCount` | **Implemented** | `CLuaGUIDefs.cpp:105` | `GUIGridListGetSelectedCount` | Operational on Android. (GuiGridListGetSelectedCount) |
| `guiGridListGetSelectedItem` | **Implemented** | `CLuaGUIDefs.cpp:103` | `GUIGridListGetSelectedItem` | Operational on Android. (GuiGridListGetSelectedItem) |
| `guiGridListGetSelectedItems` | **Implemented** | `CLuaGUIDefs.cpp:104` | `GUIGridListGetSelectedItems` | Operational on Android. (GuiGridListGetSelectedItems) |
| `guiGridListGetSelectionMode` | **Implemented** | `CLuaGUIDefs.cpp:102` | `GUIGridListGetSelectionMode` | Operational on Android. (GuiGridListGetSelectionMode) |
| `guiGridListGetVerticalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:110` | `GUIGridListGetVerticalScrollPosition` | Operational on Android. (GuiGridListGetVerticalScrollPosition) |
| `guiGridListInsertRowAfter` | **Implemented** | `CLuaGUIDefs.cpp:91` | `GUIGridListInsertRowAfter` | Operational on Android. (GuiGridListInsertRowAfter) |
| `guiGridListIsSortingEnabled` | **Implemented** | `CLuaGUIDefs.cpp:80` | `GUIGridListIsSortingEnabled` | Operational on Android. (GuiGridListIsSortingEnabled) |
| `guiGridListRemoveColumn` | **Implemented** | `CLuaGUIDefs.cpp:82` | `GUIGridListRemoveColumn` | Operational on Android. (GuiGridListRemoveColumn) |
| `guiGridListRemoveRow` | **Implemented** | `CLuaGUIDefs.cpp:92` | `GUIGridListRemoveRow` | Operational on Android. (GuiGridListRemoveRow) |
| `guiGridListSetColumnTitle` | **Implemented** | `CLuaGUIDefs.cpp:85` | `GUIGridListSetColumnTitle` | Operational on Android. (GuiGridListSetColumnTitle) |
| `guiGridListSetColumnWidth` | **Implemented** | `CLuaGUIDefs.cpp:83` | `GUIGridListSetColumnWidth` | Operational on Android. (GuiGridListSetColumnWidth) |
| `guiGridListSetHorizontalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:107` | `GUIGridListSetHorizontalScrollPosition` | Operational on Android. (GuiGridListSetHorizontalScrollPosition) |
| `guiGridListSetItemColor` | **Implemented** | `CLuaGUIDefs.cpp:99` | `GUIGridListSetItemColor` | Operational on Android. (GuiGridListSetItemColor) |
| `guiGridListSetItemData` | **Implemented** | `CLuaGUIDefs.cpp:97` | `GUIGridListSetItemData` | Operational on Android. (GuiGridListSetItemData) |
| `guiGridListSetItemText` | **Implemented** | `CLuaGUIDefs.cpp:95` | `GUIGridListSetItemText` | Operational on Android. (GuiGridListSetItemText) |
| `guiGridListSetScrollBars` | **Implemented** | `CLuaGUIDefs.cpp:87` | `GUIGridListSetScrollBars` | Operational on Android. (GuiGridListSetScrollBars) |
| `guiGridListSetSelectedItem` | **Implemented** | `CLuaGUIDefs.cpp:106` | `GUIGridListSetSelectedItem` | Operational on Android. (GuiGridListSetSelectedItem) |
| `guiGridListSetSelectionMode` | **Implemented** | `CLuaGUIDefs.cpp:101` | `GUIGridListSetSelectionMode` | Operational on Android. (GuiGridListSetSelectionMode) |
| `guiGridListSetSortingEnabled` | **Implemented** | `CLuaGUIDefs.cpp:79` | `GUIGridListSetSortingEnabled` | Operational on Android. (GuiGridListSetSortingEnabled) |
| `guiGridListSetVerticalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:109` | `GUIGridListSetVerticalScrollPosition` | Operational on Android. (GuiGridListSetVerticalScrollPosition) |
| `guiLabelGetColor` | **Implemented** | `CLuaGUIDefs.cpp:174` | `GUILabelGetColor` | Operational on Android. (GuiLabelGetColor) |
| `guiLabelGetFontHeight` | **Implemented** | `CLuaGUIDefs.cpp:179` | `GUILabelGetFontHeight` | Operational on Android. (GuiLabelGetFontHeight) |
| `guiLabelGetTextExtent` | **Implemented** | `CLuaGUIDefs.cpp:178` | `GUILabelGetTextExtent` | Operational on Android. (GuiLabelGetTextExtent) |
| `guiLabelSetColor` | **Implemented** | `CLuaGUIDefs.cpp:173` | `GUILabelSetColor` | Operational on Android. (GuiLabelSetColor) |
| `guiLabelSetHorizontalAlign` | **Implemented** | `CLuaGUIDefs.cpp:176` | `GUILabelSetHorizontalAlign` | Operational on Android. (GuiLabelSetHorizontalAlign) |
| `guiLabelSetVerticalAlign` | **Implemented** | `CLuaGUIDefs.cpp:175` | `GUILabelSetVerticalAlign` | Operational on Android. (GuiLabelSetVerticalAlign) |
| `guiMemoGetCaretIndex` | **Implemented** | `CLuaGUIDefs.cpp:167` | `GUIMemoGetCaretIndex` | Operational on Android. (GuiMemoGetCaretIndex) |
| `guiMemoGetVerticalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:171` | `GUIMemoGetVerticalScrollPosition` | Operational on Android. (GetMemoScroll) |
| `guiMemoIsReadOnly` | **Implemented** | `CLuaGUIDefs.cpp:169` | `GUIMemoIsReadOnly` | Operational on Android. (GuiMemoIsReadOnly) |
| `guiMemoSetCaretIndex` | **Implemented** | `CLuaGUIDefs.cpp:166` | `GUIMemoSetCaretIndex` | Operational on Android. (GuiMemoSetCaretIndex) |
| `guiMemoSetReadOnly` | **Implemented** | `CLuaGUIDefs.cpp:168` | `GUIMemoSetReadOnly` | Operational on Android. (GuiMemoSetReadOnly) |
| `guiMemoSetVerticalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:170` | `GUIMemoSetVerticalScrollPosition` | Operational on Android. (SetMemoScroll) |
| `guiMoveToBack` | **Implemented** | `CLuaGUIDefs.cpp:131` | `GUIMoveToBack` | Operational on Android. (GuiMoveToBack) |
| `guiProgressBarGetProgress` | **Implemented** | `CLuaGUIDefs.cpp:153` | `GUIProgressBarGetProgress` | Operational on Android. (GuiProgressBarGetProgress) |
| `guiProgressBarSetProgress` | **Implemented** | `CLuaGUIDefs.cpp:152` | `GUIProgressBarSetProgress` | Operational on Android. (GuiProgressBarSetProgress) |
| `guiRadioButtonGetSelected` | **Implemented** | `CLuaGUIDefs.cpp:150` | `GUIRadioButtonGetSelected` | Operational on Android. (GuiRadioButtonGetSelected) |
| `guiRadioButtonSetSelected` | **Implemented** | `CLuaGUIDefs.cpp:136` | `GUIRadioButtonSetSelected` | Operational on Android. (GuiRadioButtonSetSelected) |
| `guiScrollBarGetScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:119` | `GUIScrollBarGetScrollPosition` | Operational on Android. (GuiScrollBarGetScrollPosition) |
| `guiScrollBarSetScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:118` | `GUIScrollBarSetScrollPosition` | Operational on Android. (GuiScrollBarSetScrollPosition) |
| `guiScrollPaneGetHorizontalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:114` | `GUIScrollPaneGetHorizontalScrollPosition` | Operational on Android. (GuiScrollPaneGetHorizontalScrollPosition) |
| `guiScrollPaneGetVerticalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:116` | `GUIScrollPaneGetVerticalScrollPosition` | Operational on Android. (GuiScrollPaneGetVerticalScrollPosition) |
| `guiScrollPaneSetHorizontalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:113` | `GUIScrollPaneSetHorizontalScrollPosition` | Operational on Android. (GuiScrollPaneSetHorizontalScrollPosition) |
| `guiScrollPaneSetScrollBars` | **Implemented** | `CLuaGUIDefs.cpp:112` | `GUIScrollPaneSetScrollBars` | Operational on Android. (GuiScrollPaneSetScrollBars) |
| `guiScrollPaneSetVerticalScrollPosition` | **Implemented** | `CLuaGUIDefs.cpp:115` | `GUIScrollPaneSetVerticalScrollPosition` | Operational on Android. (GuiScrollPaneSetVerticalScrollPosition) |
| `guiSetAlpha` | **Implemented** | `CLuaGUIDefs.cpp:123` | `GUISetAlpha` | Operational on Android. (GuiSetAlpha) |
| `guiSetEnabled` | **Implemented** | `CLuaGUIDefs.cpp:121` | `GUISetEnabled` | Operational on Android. (GuiSetEnabled) |
| `guiSetFont` | **Implemented** | `CLuaGUIDefs.cpp:125` | `GUISetFont` | Operational on Android. (GuiSetFont) |
| `guiSetInputEnabled` | **Implemented** | `CLuaGUIDefs.cpp:41` | `GUISetInputEnabled` | Operational on Android. (GuiSetInputEnabled) |
| `guiSetInputMode` | **Implemented** | `CLuaGUIDefs.cpp:43` | `GUISetInputMode` | Operational on Android. (GuiSetInputMode) |
| `guiSetPosition` | **Implemented** | `CLuaGUIDefs.cpp:127` | `GUISetPosition` | Operational on Android. (GuiSetPosition) |
| `guiSetProperty` | **Implemented** | `CLuaGUIDefs.cpp:122` | `GUISetProperty` | Operational on Android. (GuiSetProperty) |
| `guiSetSelectedTab` | **Implemented** | `CLuaGUIDefs.cpp:76` | `GUISetSelectedTab` | Operational on Android. (GuiSetSelectedTab) |
| `guiSetSize` | **Implemented** | `CLuaGUIDefs.cpp:126` | `GUISetSize` | Operational on Android. (GuiSetSize) |
| `guiSetText` | **Implemented** | `CLuaGUIDefs.cpp:124` | `GUISetText` | Operational on Android. (GuiSetText) |
| `guiSetVisible` | **Implemented** | `CLuaGUIDefs.cpp:128` | `GUISetVisible` | Operational on Android. (GuiSetVisible) |
| `guiStaticImageGetNativeSize` | **Implemented** | `CLuaGUIDefs.cpp:74` | `GUIStaticImageGetNativeSize` | Operational on Android. (GuiStaticImageGetNativeSize) |
| `guiStaticImageLoadImage` | **Implemented** | `CLuaGUIDefs.cpp:73` | `GUIStaticImageLoadImage` | Operational on Android. (GuiStaticImageLoadImage) |
| `guiWindowIsMovable` | **Implemented** | `CLuaGUIDefs.cpp:183` | `GUIWindowIsMovable` | Operational on Android. (GuiWindowIsMovable) |
| `guiWindowIsSizable` | **Implemented** | `CLuaGUIDefs.cpp:184` | `GUIWindowIsSizable` | Operational on Android. (GuiWindowIsSizable) |
| `guiWindowSetMovable` | **Implemented** | `CLuaGUIDefs.cpp:181` | `GUIWindowSetMovable` | Operational on Android. (GuiWindowSetMovable) |
| `guiWindowSetSizable` | **Implemented** | `CLuaGUIDefs.cpp:182` | `GUIWindowSetSizable` | Operational on Android. (GuiWindowSetSizable) |
| `isChatBoxInputActive` | *Stub* | `CLuaGUIDefs.cpp:45` | `GUIIsChatBoxInputActive` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `isConsoleActive` | *Stub* | `CLuaGUIDefs.cpp:46` | `GUIIsConsoleActive` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isDebugViewActive` | Missing | `CLuaGUIDefs.cpp:47` | `GUIIsDebugViewActive` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isMTAWindowActive` | Missing | `CLuaGUIDefs.cpp:50` | `GUIIsMTAWindowActive` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isMainMenuActive` | *Stub* | `CLuaGUIDefs.cpp:49` | `GUIIsMainMenuActive` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isTransferBoxActive` | Missing | `CLuaGUIDefs.cpp:51` | `GUIIsTransferBoxActive` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `removeModel2DFX` | Missing | `CLua2DFXDefs.cpp:568` | `RemoveModel2DFX` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `resetModel2DFXEffects` | Missing | `CLua2DFXDefs.cpp:569` | `ResetModel2DFXEffects` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `resetModel2DFXPosition` | Missing | `CLua2DFXDefs.cpp:571` | `ResetModel2DFXPosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `resetModel2DFXProperty` | Missing | `CLua2DFXDefs.cpp:570` | `ResetModel2DFXProperty` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `restoreModel2DFX` | Missing | `CLua2DFXDefs.cpp:568` | `RestoreModel2DFX` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setChatboxCharacterLimit` | Missing | `CLuaGUIDefs.cpp:53` | `ArgumentParser<GUISetChatboxCharacterLimit>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setDebugViewActive` | Missing | `CLuaGUIDefs.cpp:48` | `GUISetDebugViewActive` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setModel2DFXPosition` | Missing | `CLua2DFXDefs.cpp:569` | `SetModel2DFXPosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setModel2DFXProperty` | Missing | `CLua2DFXDefs.cpp:570` | `SetModel2DFXProperty` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `svgCreate` | Missing | `CLuaVectorGraphicDefs.cpp:17` | `ArgumentParser<SVGCreate>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `svgGetDocumentXML` | Missing | `CLuaVectorGraphicDefs.cpp:18` | `ArgumentParser<SVGGetDocumentXML>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `svgGetSize` | Missing | `CLuaVectorGraphicDefs.cpp:20` | `ArgumentParser<SVGGetSize>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `svgSetDocumentXML` | Missing | `CLuaVectorGraphicDefs.cpp:19` | `ArgumentParser<SVGSetDocumentXML>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `svgSetSize` | Missing | `CLuaVectorGraphicDefs.cpp:21` | `ArgumentParser<SVGSetSize>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `svgSetUpdateCallback` | Missing | `CLuaVectorGraphicDefs.cpp:22` | `ArgumentParser<SVGSetUpdateCallback>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
