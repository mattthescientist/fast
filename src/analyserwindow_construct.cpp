// The FTS Atomic Spectrum Tool (FAST)
// Copyright (C) 2011-2013 M. P. Ruffoni
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//==============================================================================
// AnalyserWindow class (analyserwindow_construct.cpp)
//==============================================================================
// This file contains the class constructor and destructor. The constructor
// needs to prepare many GTK+ widgets, and so is very long.

#include "XGremlin.xpm"
#include "Targets.xpm"

//------------------------------------------------------------------------------
// default constructor : prepares all the GTK widgets and packs them into the
// AnalyserWindow. Finishes by showing all the child widgets and then the
// AnalyserWindow itself.
//
AnalyserWindow::AnalyserWindow () {
  Gtk::CellRendererText* pRenderer;
  Gtk::TreeViewColumn* pColumn;
  int Col;

  set_title ("FAST");
  set_default_size (800, 600);
  maximize ();
  Status.push ("Welcome to the FTS Atomic Spectrum Tool (FAST)");
  ViewLineParams = false;
  CurrentFilename = "";
  DefaultFolder = "";
  ProjectChangedSinceSave = false;
  readConfigFile ();

  // Build the menubar and toolbar and add them to the top of the BaseBox
  buildMenubarAndToolbar ();
  
  // Prepare the central potion of the AnalyserWindow
  frameSpectra.set_label ("Experimental Spectra");
  frameLevels.set_label ("Target Upper Levels");
  frameSpectra.add (scrollSpectra);
  bookData.append_page (scrollDataKur, "Target Lines");
  bookData.append_page (scrollDataXGr, "Fitted Profiles");
  bookData.append_page (scrollDataBF, "Branching Fractions");
  bookData.append_page (scrollDataComp, "Compare Spectra");
  bookLevels.append_page (scrollLevels, "Levels");
  bookLevels.append_page (scrollLevelsBF, "Extra Data");
  frameLevels.add (bookLevels);
  scrollSpectra.add (treeSpectra);
  scrollLevels.add (treeLevels);
  scrollLevelsBF.add (treeLevelsBF);
  scrollDataKur.add (treeDataKur);
  scrollDataXGr.add (treeDataXGr);
  scrollDataComp.add (treeDataComp);
  scrollDataBF.add (treeDataBF);
  ProfileScroll.add (Profiles);
  
  // Prepare the resizable window panes
  int width, height;
  this -> get_size (width, height);
  vpanedLeftDivider.set_position (height * 0.77);
  vpanedRightDivider.set_position (height * 0.77);
  hpanedDivider.set_position (width * 0.52);    
  vpanedLeftDivider.add1 (frameLevels);
  vpanedLeftDivider.add2 (frameSpectra);
  vpanedRightDivider.add1 (ProfileScroll);
  vpanedRightDivider.add2 (bookData);
  hpanedDivider.add1 (vpanedLeftDivider);
  hpanedDivider.add2 (vpanedRightDivider);

  // Only show scrollbars if necessary
  scrollSpectra.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrollLevels.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  ProfileScroll.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrollDataKur.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  
  // Populate the BaseBox (the main VBox) with the primary window Widgets
  BaseBox.pack_start (hpanedDivider, true, true, 0);
  BaseBox.pack_start (Status, false, false, 0);
  add (BaseBox);
  
  // Create the tree models for the treeView objects
  m_refTreeModel = Gtk::TreeStore::create (m_Columns);
  levelTreeModel = Gtk::TreeStore::create (levelCols);
  modelLevelsBF = Gtk::TreeStore::create (levelColsBF);
  lineDataTreeModel = Gtk::TreeStore::create (targetsCols);
  modelDataXGr = Gtk::TreeStore::create (colsDataXGr);
  modelDataComp = Gtk::TreeStore::create (colsDataComp);
  modelDataBF = Gtk::TreeStore::create (colsDataBF);

  // Add columns to the Upper Levels tree
  treeLevels.set_model (levelTreeModel);
  treeLevels.append_column_numeric ("Energy (cm^-1)", levelCols.eupper, "%11.3f");
  treeLevels.append_column ("Config.", levelCols.config);
  treeLevels.append_column_numeric ("J", levelCols.jupper, "%3.1f");
  columnLifetime.set_title("t (ns)");
  columnLifetime.pack_start(textLifetime);
  columnLifetime.add_attribute(textLifetime.property_text(), levelCols.lifetime);
  columnLifetime.add_attribute(textLifetime.property_background_gdk(), levelCols.lifetime_colour);
  treeLevels.append_column(columnLifetime);
  columnLifetimeError.set_title("U(t) (ns)");
  columnLifetimeError.pack_start(textLifetimeError);
  columnLifetimeError.add_attribute(textLifetimeError.property_text(), levelCols.err_lifetime);
  columnLifetimeError.add_attribute(textLifetimeError.property_background_gdk(), levelCols.err_lifetime_colour);
  treeLevels.append_column(columnLifetimeError);
  
  // Display a progress bar in the % complete column
  Gtk::CellRendererProgress* cell = Gtk::manage(new Gtk::CellRendererProgress);
  Col = treeLevels.append_column ("Complete", *cell);
  pColumn = treeLevels.get_column(Col - 1);
  if(pColumn) {
    #ifdef GLIBMM_PROPERTIES_ENABLED
      pColumn->add_attribute(cell->property_value(), levelCols.fracFound);
    #else
      pColumn->add_attribute(*cell, "value", levelCols.fracFound);
    #endif
  }
  
  // Add columns to the Upper Levels branching fractions tab
  treeLevelsBF.set_model (modelLevelsBF);
  treeLevelsBF.append_column_numeric ("Energy (cm^-1)", levelColsBF.eupper, "%11.3f");
  treeLevelsBF.append_column ("Config.", levelColsBF.config);
  treeLevelsBF.append_column_numeric ("J", levelColsBF.jupper, "%3.1f");
  treeLevelsBF.append_column ("Branches", levelColsBF.branches);
  treeLevelsBF.append_column_numeric ("A observed", levelColsBF.a_total, "%8.4e");
  treeLevelsBF.append_column_numeric ("A missing / %", levelColsBF.a_missing, "%6.3f");
  treeLevelsBF.append_column_numeric ("I observed", levelColsBF.ew_total, "%8.4e");
  treeLevelsBF.append_column_numeric ("I normalised", levelColsBF.ew_norm, "%6.3f");

  // Generate custom cell renderers for the I columns so they may be coloured 
  // depending on whether or not the spectrum is intensity calibrated.
/*  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeLevelsBF.append_column ("I observed", *pRenderer);
  pColumn = treeLevelsBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), levelColsBF.ew_total);
  pColumn->add_attribute(pRenderer->property_background_gdk(), levelColsBF.bg_colour);
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeLevelsBF.append_column ("I normalised", *pRenderer);
  pColumn = treeLevelsBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), levelColsBF.ew_norm);
  pColumn->add_attribute(pRenderer->property_background_gdk(), levelColsBF.bg_colour);*/

  // Prepare the text input column for the level lifetime
  columnLifetime.set_cell_data_func(textLifetime, sigc::mem_fun
    (*this, &AnalyserWindow::lifetimeValidatedOnCellData));
  columnLifetimeError.set_cell_data_func(textLifetimeError, sigc::mem_fun
    (*this, &AnalyserWindow::lifetimeErrorValidatedOnCellData));
  #ifdef GLIBMM_PROPERTIES_ENABLED
    textLifetime.property_editable() = true;    
    textLifetimeError.property_editable() = true;
  #else
    textLifetime.set_property("editable", true);
    textLifetimeError.set_property("editable", true);
  #endif
  textLifetime.signal_editing_started().connect(
    sigc::mem_fun(*this, &AnalyserWindow::lifetimeEditStarted));
  textLifetime.signal_edited().connect(
    sigc::mem_fun(*this, &AnalyserWindow::lifetimeEdited));
  textLifetimeError.signal_editing_started().connect(
    sigc::mem_fun(*this, &AnalyserWindow::lifetimeEditStarted));
  textLifetimeError.signal_edited().connect(
    sigc::mem_fun(*this, &AnalyserWindow::lifetimeErrorEdited));

  // Add columns to the Experimental Spectra tree. Use custom cell renderers to
  // allow the columns to be coloured depending on what type of item is shown.
  treeSpectra.set_model (m_refTreeModel);
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeSpectra.append_column (" ", *pRenderer);
  pColumn = treeSpectra.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), m_Columns.label);
  pColumn->add_attribute(pRenderer->property_cell_background_gdk(), m_Columns.bg_colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeSpectra.append_column ("Spectrum", *pRenderer);
  pColumn = treeSpectra.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), m_Columns.name);
  pColumn->add_attribute(pRenderer->property_background_gdk(), m_Columns.bg_colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeSpectra.append_column ("E min", *pRenderer);
  pColumn = treeSpectra.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), m_Columns.emin);
  pColumn->add_attribute(pRenderer->property_background_gdk(), m_Columns.bg_colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeSpectra.append_column ("E max", *pRenderer);
  pColumn = treeSpectra.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), m_Columns.emax);
  pColumn->add_attribute(pRenderer->property_background_gdk(), m_Columns.bg_colour);

  Gtk::CellRendererToggle* refRenderer = Gtk::manage (new Gtk::CellRendererToggle());
  refRenderer->signal_toggled().connect (sigc::mem_fun(*this, &AnalyserWindow::ref_spectrum_toggled));
  Col = treeSpectra.append_column ("Ref", *refRenderer);
  pColumn = treeSpectra.get_column(Col - 1);
  pColumn->add_attribute(refRenderer->property_active(), m_Columns.ref);
  pColumn->add_attribute(refRenderer->property_cell_background_gdk(), m_Columns.bg_colour);
  
  // Add columns to the "XGremlin Data" tab at the bottom right of the window.
  // Use a custom cell renderer for the Eq. Width column so it can be coloured
  // depending on whether or not the spectrum is intensity calibrated.
  treeDataXGr.set_model (modelDataXGr);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Spec", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.spectrum);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Line", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.index);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Label", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.id);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Wavenumber", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.wavenumber);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Peak", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.peak);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Width", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.width);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Damping", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.dmp);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Eq. Width", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.eqwidth);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.eq_width_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Err(Tot)", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.epstot);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Err(Even)", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.epsevn);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Err(Odd)", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.epsodd);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataXGr.append_column ("Err(Rand)", *pRenderer);
  pColumn = treeDataXGr.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataXGr.epsran);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataXGr.bg_colour);

  // Add columns to the "Br. Frac. Data" tab at the bottom right of the window.
  // Use a custom cell renderer for the Eq. Width column so it can be coloured
  // depending on whether or not the spectrum is intensity calibrated.
  treeDataBF.set_model (modelDataBF);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("Spec", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.spectrum);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("Line", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.index);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("Wavenumber", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.wavenumber);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("Intensity", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.eqwidth);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.eq_width_colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("U(S/N) / %", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.err_line);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("U(Cal.) / %", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.err_cal);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.err_cal_colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("U(Trans.) / %", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.err_trans);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("U(Total) / %", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.err_total);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);
    
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("U(Int.)", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.err_eqwidth);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("BF", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.br_frac);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("U(BF) / %", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.err_br_frac);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("A", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.a);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("U(A) / %", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.err_a);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("log(gf)", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.loggf);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  
  
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataBF.append_column ("dex", *pRenderer);
  pColumn = treeDataBF.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), colsDataBF.dex);
  pColumn->add_attribute(pRenderer->property_background_gdk(), colsDataBF.bg_colour);  

  // Add columns to the "Compare Spectra" tab at the bottom right of the window.
  treeDataComp.set_model (modelDataComp);
  treeDataComp.append_column ("Reference", colsDataComp.ref);
  treeDataComp.append_column ("Compare to", colsDataComp.comparison);
  treeDataComp.append_column ("Wavenumber", colsDataComp.wavenumber);
  treeDataComp.append_column ("Ratio +/- Error", colsDataComp.ratio);
  
  // Add columns to the "Target Lines" tab at the bottom right of the window. Use
  // custom cell renderers to allow the rows to be coloured according to the
  // state of the each line.
  treeDataKur.set_model (lineDataTreeModel);
  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column("Wavenumber / K", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.sigma);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column("Wavelength / nm", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.wavelength);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column ("log(gf)", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.loggf);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column ("Br. Frac.", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.bf);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column("E lower / K", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.elower);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column ("J lower", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.jlower);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column("Config lower", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.configlower);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column("E upper / K", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.eupper);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column ("J upper", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.jupper);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  pRenderer = Gtk::manage(new Gtk::CellRendererText());
  Col = treeDataKur.append_column("Config upper", *pRenderer);
  pColumn = treeDataKur.get_column(Col - 1);
  pColumn->add_attribute(pRenderer->property_text(), targetsCols.configupper);
  pColumn->add_attribute(pRenderer->property_background_gdk(), targetsCols.bg_colour);
  pColumn->add_attribute(pRenderer->property_foreground_gdk(), targetsCols.colour);

  // Construct the treeSpectra popup menu for a right click on a spectrum.
  {
    Gtk::Menu::MenuList& menulist = menuSpectraPopup.items();
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Attach Line List",
      sigc::mem_fun(*this, &AnalyserWindow::on_data_load_line_list)));
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Attach Standard Lamp Spectrum",
      sigc::mem_fun(*this, &AnalyserWindow::on_data_attach_standard_lamp_spectrum)));
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Attach Standard Lamp Radiance",
      sigc::mem_fun(*this, &AnalyserWindow::on_data_attach_standard_lamp_radiance)));
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Link to Another Spectrum",
      sigc::mem_fun(*this, &AnalyserWindow::on_popup_link_spectrum)));
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Remove Spectrum",
      sigc::mem_fun(*this, &AnalyserWindow::on_data_remove_spectrum)));
  }
  menuSpectraPopup.accelerate(*this);
  
  // Construct the treeSpectra popup menu for a right click on a line list
  {
    Gtk::Menu::MenuList& menulist = menuLinelistPopup.items();

    menulist.push_back( Gtk::Menu_Helpers::MenuElem("Remove Line List",
      sigc::mem_fun(*this, &AnalyserWindow::on_popup_remove_linelist) ) );
    menulist.push_back( Gtk::Menu_Helpers::MenuElem("Export Line List",
      sigc::mem_fun(*this, &AnalyserWindow::on_popup_export_linelist) ) );
  }
  menuLinelistPopup.accelerate(*this);
  
  // Construct the treeSpectra popup for a right click on a response function
  {
    Gtk::Menu::MenuList& menulist = menuStdLampPopup.items();

    menulist.push_back( Gtk::Menu_Helpers::MenuElem("Remove Standard Lamp Spectrum",
      sigc::mem_fun(*this, &AnalyserWindow::on_popup_remove_standard_lamp_spectrum) ) );
  }
  menuStdLampPopup.accelerate(*this);
  
  // Construct the treeSpectra popup for a right click on a response function
  // calibration error file
  {
    Gtk::Menu::MenuList& menulist = menuRadiancePopup.items();

    menulist.push_back( Gtk::Menu_Helpers::MenuElem("Remove Standard Lamp Radiance",
      sigc::mem_fun(*this, &AnalyserWindow::on_popup_remove_radiance)));
  }
  menuRadiancePopup.accelerate(*this);
  
  
  // Construct the treeSpectrua popup menu for a right click on empty space
  {
    Gtk::Menu::MenuList& menulist = menuInitialPopup.items();
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Add _New Spectrum",
      sigc::mem_fun(*this, &AnalyserWindow::on_data_load_expt_spectrum)));
  }
  menuInitialPopup.accelerate(*this);
  
  // Construct the treeLevels popup menu for a right click on empty space
  {
    Gtk::Menu::MenuList& menulist = menuLevelInitPopup.items();
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Load Kurucz Line List",
      sigc::mem_fun(*this, &AnalyserWindow::on_data_load_kurucz)));
  }
  menuLevelInitPopup.accelerate(*this);
  
  // Construct the treeLevels popup menu for a right click on an upper level
  {
    Gtk::Menu::MenuList& menulist = menuLevelPopup.items();
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Load Kurucz Line List",
      sigc::mem_fun(*this, &AnalyserWindow::on_data_load_kurucz)));
    menulist.push_back(Gtk::Menu_Helpers::MenuElem("Remove Level",
      sigc::mem_fun(*this, &AnalyserWindow::on_popup_remove_level)));
  }
  menuLevelPopup.accelerate(*this);

  // Allow multiple selections to be made in the "XGremlin Data" tab
  Glib::RefPtr<Gtk::TreeSelection> treeDataXGrSel = treeDataXGr.get_selection();
  treeDataXGrSel->set_mode(Gtk::SELECTION_MULTIPLE);
  
  // Connect signal handlers to treeView objects that require them
  treeDataXGrSel->signal_changed().connect
    (sigc::mem_fun(*this, &AnalyserWindow::on_click_treeDataXGr));
  treeSpectra.signal_button_press_event().connect_notify
    (sigc::mem_fun(*this, &AnalyserWindow::on_click_treeSpectra));
  treeLevels.signal_button_press_event().connect_notify
    (sigc::mem_fun(*this, &AnalyserWindow::on_click_level_list));
  treeLevelsBF.signal_button_press_event().connect_notify
    (sigc::mem_fun(*this, &AnalyserWindow::on_click_level_list_bf));

  // Show all the widgets then display the window itself
  show_all_children ();
  show ();
}


//------------------------------------------------------------------------------
// default destructor : Does nothing
//
AnalyserWindow::~AnalyserWindow () {

}


//------------------------------------------------------------------------------
// buildMenubar () : Constructs the menubar for the AnalyserWindow. This func is
// called from the class constructor when a new AnalyserWindow is created.
//
void AnalyserWindow::buildMenubarAndToolbar (bool PackWidgets) {
  ostringstream oss;
  unsigned int Separator;

  // Create the "File" menu
  m_refActionGroup = Gtk::ActionGroup::create();
  m_refActionGroup->add( Gtk::Action::create("FileMenu", "_File") );
  m_refActionGroup->add( Gtk::Action::create("FileNew", Gtk::Stock::FILE, 
    "_New", "Create a new project"), Gtk::AccelKey ("<control>n"),
    sigc::mem_fun(this, &AnalyserWindow::on_file_new) );
  m_refActionGroup->add( Gtk::Action::create("FileOpen", Gtk::Stock::OPEN),
    sigc::mem_fun(this, &AnalyserWindow::on_file_open) );
  m_refActionGroup->add( Gtk::Action::create("FileSave", Gtk::Stock::SAVE),
    sigc::mem_fun(this, &AnalyserWindow::on_file_save) );
  m_refActionGroup->get_action("FileSave")->set_sensitive(false);
  m_refActionGroup->add( Gtk::Action::create("FileSaveAs", Gtk::Stock::SAVE_AS),
    Gtk::AccelKey ("<control><shift>s"),
    sigc::mem_fun(this, &AnalyserWindow::on_file_save_as) );
  m_refActionGroup->add( Gtk::Action::create("FileExportProject", 
    "Export Project"), Gtk::AccelKey ("<control>e"),
    sigc::mem_fun(this, &AnalyserWindow::on_file_export_project) );
  m_refActionGroup->add( Gtk::Action::create("FilePrintLevel", Gtk::Stock::PRINT,
    "Print Level", "Prints the currently selected level"),
    sigc::mem_fun(this, &AnalyserWindow::on_file_print_level) );
  for (unsigned int i = 0; i < RecentFiles.size (); i ++) {
	oss.str ("");
	oss << "Recent" << i;
	Separator = RecentFiles[i].find_last_of("/\\");
    m_refActionGroup->add( Gtk::Action::create(oss.str().c_str(), RecentFiles[i].substr (Separator + 1)),
      sigc::bind(sigc::mem_fun(this, &AnalyserWindow::fileOpen), RecentFiles[i]) );
  }
  m_refActionGroup->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT,
    "_Quit", "Quit the program. Unsaved data will be lost."),
    sigc::mem_fun(this, &AnalyserWindow::on_file_quit) );

  // Create the "Data" menu
  m_refActionGroup->add( Gtk::Action::create("DataMenu", "_Data") );
  m_refActionGroup->add( Gtk::Action::create("LoadTarget", "Load Target Lines"),
    sigc::mem_fun(this, &AnalyserWindow::on_data_load_kurucz) );
  m_refActionGroup->add( Gtk::Action::create("SaveTarget", "Save Target Line List"),
    sigc::mem_fun(this, &AnalyserWindow::on_data_save_kurucz) );
  m_refActionGroup->add( Gtk::Action::create("AddLineList", "Attach Experimental Line List"),
    sigc::mem_fun(this, &AnalyserWindow::on_data_load_line_list) );
  m_refActionGroup->add( Gtk::Action::create("AddExptSpectum", "Add Experimental Spectrum"),
    sigc::mem_fun(this, &AnalyserWindow::on_data_load_expt_spectrum) );
  m_refActionGroup->add( Gtk::Action::create("OutputResults", "Output results"),
    sigc::mem_fun(this, &AnalyserWindow::on_data_output_results) );
  
  // Create the "Tools" menu
  m_refActionGroup->add( Gtk::Action::create("ToolsMenu", "_Tools") );
  m_refActionGroup->add( Gtk::Action::create("Options", "Options",
    "Allows a number of FAST options to be changed"),
    sigc::mem_fun(this, &AnalyserWindow::on_tools_options) );
  
  // Create the "Help" menu
  m_refActionGroup->add( Gtk::Action::create("HelpMenu", "_Help") );
  m_refActionGroup->add( Gtk::Action::create("About", "About FAST",
    "Shows some basic information about FAST"),
    sigc::mem_fun(this, &AnalyserWindow::on_help_about) );

  // Add XPM icons for the toolbar
  add_stock_item(Targets_xpm, "targets", "Add Target Lines");
  add_stock_item(Xgremlin_xpm, "spectra", "Add Spectral Data");
  add_stock_item(Targets_xpm, "output",  "Output Results");
  
  // Create toolbar items
  m_refActionGroup->add( Gtk::Action::create("ToolLoadTarget", 
    Gtk::StockID("targets"), "1. Add Target Lines",
    "Specifies which lines will be targeted by the current project"),
    sigc::mem_fun(this, &AnalyserWindow::on_data_load_kurucz) );
  m_refActionGroup->add( Gtk::Action::create("ToolLoadExperiment", 
    Gtk::StockID("spectra"), "2. Add Spectral Data",
    "Adds a new experimental spectrum and line list to the project"),
    sigc::mem_fun(this, &AnalyserWindow::loadXGremlinData) );
  m_refActionGroup->add( Gtk::Action::create("ToolOutputResults", 
    Gtk::StockID("targets"), "3. Output Results",
    "Outputs results for further analysis or publication"),
    sigc::mem_fun(this, &AnalyserWindow::on_data_output_results) );
    
  // Add the menus to a UI manager object
  m_refUIManager = Gtk::UIManager::create();
  m_refUIManager->insert_action_group(m_refActionGroup);
  add_accel_group(m_refUIManager->get_accel_group());

  //Layout the actions in a menubar and toolbar:
  string MenuInterface =
		"<ui>"
		"  <menubar name='MenuBar'>"
		"    <menu action='FileMenu'>"
		"      <menuitem action='FileNew'/>"
		"      <menuitem action='FileOpen'/>"
		"      <menuitem action='FileSave'/>"
		"      <menuitem action='FileSaveAs'/>"
		"      <menuitem action='FileExportProject'/>"
		"      <separator action='sep'/>";
  for (unsigned int i = 0; i < RecentFiles.size (); i ++) {
	  oss.str ("");
	  oss << "      <menuitem action='Recent" << i << "'/>";
	  MenuInterface += oss.str ();
  }
  MenuInterface +=
  		"      <separator action='sep'/>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu action='DataMenu'>"
        "      <menuitem action='LoadTarget'/>"
        "      <menuitem action='AddExptSpectum'/>"
        "      <menuitem action='AddLineList'/>"
        "      <separator action='sep'/>"
        "      <menuitem action='OutputResults'/>"
        "    </menu>"
        "    <menu action='ToolsMenu'>"
        "      <menuitem action='Options'/>"
        "    </menu>"
        "    <menu action='HelpMenu'>"
        "      <menuitem action='About'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar name = 'ToolBar'>"
        "    <toolitem action='FileNew'/>"
        "    <toolitem action='FileOpen'/>"
        "    <toolitem action='FileSave'/>"

        "    <separator action='sep'/>"
        "    <toolitem action='ToolLoadTarget'/>"
        "    <toolitem action='ToolLoadExperiment'/>"
        "    <toolitem action='ToolOutputResults'/>"
        "    <separator action='sep'/>"
        "  </toolbar>"
        "</ui>";
  Glib::ustring ui_info = MenuInterface.c_str();

  // Build the menus
  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
  #else
  std::auto_ptr<Glib::Error> ex;
  m_refUIManager->add_ui_from_string(ui_info, ex);
  if(ex.get())
  {
    std::cerr << "building menus failed: " <<  ex->what();
  }
  #endif //GLIBMM_EXCEPTIONS_ENABLED
  
  // Create widgets for both the menubar and toolbar, then pack them
  pMenubar = m_refUIManager->get_widget("/MenuBar");
  if (pMenubar) BaseBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);
  if (PackWidgets) {
	  Gtk::Toolbar* pToolbar = dynamic_cast<Gtk::Toolbar*>(m_refUIManager->get_widget("/ToolBar"));
	  if (pToolbar) {
		BaseBox.pack_start(*pToolbar, Gtk::PACK_SHRINK);
		pToolbar -> set_toolbar_style (Gtk::TOOLBAR_BOTH);
	  }
  }
}

void AnalyserWindow::add_stock_item(const char *name[], Glib::ustring id, Glib::ustring label)
{
  Glib::RefPtr<Gtk::IconFactory> factory = Gtk::IconFactory::create();
  factory->add_default(); //Add factory to list of factories.

  Gtk::IconSource source;
  try
  {
    //This throws an exception if the file is not found:
    source.set_pixbuf( Gdk::Pixbuf::create_from_xpm_data(name) );
  }
  catch(const Glib::Exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  source.set_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
  source.set_size_wildcarded(); //Icon may be scaled.

  Gtk::IconSet icon_set;
  icon_set.add_source(source); //More than one source per set is allowed.

  const Gtk::StockID stock_id(id);
  factory->add(stock_id, icon_set);
  Gtk::Stock::add(Gtk::StockItem(stock_id, label));
}

