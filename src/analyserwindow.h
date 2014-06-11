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
// AnalyserWindow class (analyserwindow.h)
//==============================================================================
// Describes the main GTK window for FAST. It contains all the widgets displayed
// in the window and all the signal handlers required to process user actions.
// Since there are many class functions, they have been divided into separate
// source files based on their purpose:
//
//   analyserwindow.cpp              : Contains general functions that don't fit
//                                     neatly into any given category
//   analyserwindow_construct.cpp    : Class constructors and destructors
//   analyserwindow_errors.cpp       : Contains AnalyserWindow error handlers
//   analyserwindow_io.cpp           : Contains File I/O functions
//   analyserwindow_refresh.cpp      : Functions for refresing data that
//                                     is to be displayed in the FAST interface
//   analyserwindow_signal.cpp       : General interface signal handlers
//   analyserwindow_signal_file.cpp  : Signal handlers for the UI File menu
//   analyserwindow_signal_data.cpp  : Signal handlers for the UI Data menu
//   analyserwindow_signal_click.cpp : Signal handlers for UI click events
//   analyserwindow_signal_popup.cpp : Signal handlers for UI popup menus
//
#ifndef LINE_ANALYSER_WINDOW
#define LINE_ANALYSER_WINDOW

// Include the GTK+ environment from GTKmm header files
#include <gtkmm/window.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/dialog.h>
#include <gtkmm/table.h>
#include <gtkmm/cellrendererprogress.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/notebook.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/paned.h>
#include <gtkmm/iconfactory.h>
#include <sys/stat.h>
#include <gtkmm/main.h>
#include <cstdio>
#if defined (_WIN32)
  #include <direct.h>
#endif

// Include other elements of FAST
#include "TypeDefs.h"
#include "ErrDefs.h"
#include "kzlist.h"
#include "xgline.h"
#include "graph.h"
#include "linedata.h"
#include "voigtlsqfit.h"
#include "xgspectrum.h"
#include "about.h"
#include "outputwindow.h"
#include "optionswindow.h"

using namespace::std;

// Model for treeLevels, which contains a list of all the target upper
// levels loaded from a Kurucz line list.
class LevelColumns : public Gtk::TreeModel::ColumnRecord {
  public:
    Gtk::TreeModelColumn<int> index;
    Gtk::TreeModelColumn<string> name, config;
    Gtk::TreeModelColumn<double> eupper, jupper, fracFound, lifetime, err_lifetime;
    Gtk::TreeModelColumn<Gdk::Color> lifetime_colour, err_lifetime_colour;
    LevelColumns() { add (index); add (lifetime); add (err_lifetime); add (name);
      add (jupper); add (config); add (eupper); add (fracFound); 
      add (lifetime_colour); add (err_lifetime_colour); }
};


class AnalyserWindow : public Gtk::Window {

  private:
    // Class variables to store all loaded Kurucz and XGremlin data
    KzList KuruczList;
    vector < XgSpectrum > ExptSpectra;
    vector < vector < vector <LinePair> > > LevelLines;
    vector < vector <LineData *> > LineBoxes; 
    vector < Gtk::Frame *> frameSpectrumPlots;
    vector < Gtk::HBox *> hboxSpectrumPlots;
    vector <RatioAndError> ScalingFactors;
    bool ViewLineParams;
    string CurrentFilename;
    string DefaultFolder;
    bool ProjectChangedSinceSave;
    typedef struct type_link_spectra { unsigned int a, b; } TypeLinkSpectra;
    vector <TypeLinkSpectra> LinkedSpectra;
    sigc::connection LinkConnection, AbortLinkConnection;
    OutputWindow Output;
    OptionsWindow Options;

    // GTKmm VBox to hold all the widgets in the AnalyserWindow
    Gtk::VBox BaseBox;

    // GTKmm Menubar. Declared here so the recent file list can be updated
    Gtk::Widget* pMenubar;

    // Primary GTKmm Widgets for the AnalyserWindow
    Gtk::Frame frameSpectra;           // Contains bookSpectra
    Gtk::Frame frameLevels;            // Contains scrollLevels
    Gtk::Notebook bookData;            // Provides tabs for selecting various line data
    Gtk::Notebook bookLevels;          // Provides tabs for selecting various level data
    Gtk::VPaned vpanedLeftDivider;     // Movable vertical divider on the left of the window
    Gtk::VPaned vpanedRightDivider;    // Movable vertical divider on the right of the window
    Gtk::HPaned hpanedDivider;         // Movable horizontal divider containing vpanedLeftDivider and vpanedRightDivider
    Gtk::ScrolledWindow scrollSpectra; // Contains treeSpectra
    Gtk::ScrolledWindow scrollLevels;  // Contains treeLevels
    Gtk::ScrolledWindow scrollLevelsBF;// Contains treeLevelsBF
    Gtk::ScrolledWindow scrollDataKur; // Contains treeLineData
    Gtk::ScrolledWindow scrollDataXGr; // Contains treeDataXGr
    Gtk::ScrolledWindow scrollDataComp;// Contains treeDataComp
    Gtk::ScrolledWindow scrollDataBF;  // Contains treeDataBF
    Gtk::ScrolledWindow ProfileScroll; // Contains the Profiles table
    Gtk::TreeView treeSpectra;         // Contains all the XGremlin spectra (ASCII) and line lists
    Gtk::TreeView treeLevels;          // Contains all upper levels loaded from a Kurucz list
    Gtk::TreeView treeLevelsBF;        // Contains BF data for loaded upper levels
    Gtk::TreeView treeDataKur;         // Contains the Kurucz lines for the selected upper level
    Gtk::TreeView treeDataXGr;         // Contains the XGremlin lines for the selected upper level
    Gtk::TreeView treeDataComp;        // Contains information for the comparison of loaded experimental spectra
    Gtk::TreeView treeDataBF;          // Contains BF and log(gf) data for each line in the selected upper level
    Gtk::VBox Profiles;                // Displays plots for XGremlin lines corresponding to the selected upper level
    Gtk::Statusbar Status;

    // Properties for creating an editable text cell for level lifetimes
    Gtk::CellRendererText textLifetime, textLifetimeError;
    Gtk::TreeView::Column columnLifetime, columnLifetimeError;
    bool lifetimeValidated, lifetimeErrorValidated;
    Glib::ustring invalid_text_for_retry;

    // Popup context menus accessed through right-mouse clicks
    Gtk::Menu menuSpectraPopup;			// For a click on a line spectrum listed in treeSpectra
    Gtk::Menu menuLinelistPopup;		// For a click on a line list in treeSpectra
    Gtk::Menu menuStdLampPopup;			// For a click on a standard lamp spectrum in treeSpectra
    Gtk::Menu menuRadiancePopup;		// For a click on a standard lamp RAD file in treeSpectra
    Gtk::Menu menuInitialPopup;			// For a click on a black area of treeSpectra
    Gtk::Menu menuLevelInitPopup;		// For a click on a blank area of treeLevels
    Gtk::Menu menuLevelPopup;			// For a click on a level in treeLevels

    // Glib pointers to treeView data
    Glib::RefPtr<Gtk::UIManager> m_refUIManager;
    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
    Glib::RefPtr<Gtk::TreeStore> m_refTreeModel;
    Glib::RefPtr<Gtk::TreeStore> levelTreeModel;
    Glib::RefPtr<Gtk::TreeStore> modelLevelsBF;
    Glib::RefPtr<Gtk::TreeStore> lineDataTreeModel;
    Glib::RefPtr<Gtk::TreeStore> modelDataXGr;
    Glib::RefPtr<Gtk::TreeStore> modelDataComp;
    Glib::RefPtr<Gtk::TreeStore> modelDataBF;

    // List of recently loaded .FTS files for the File toolbar menu
    vector <string> RecentFiles;
        
    // Private class methods
    void readConfigFile ();
    void writeConfigFile ();
    void removeFromConfigFile (string Filename);
    void buildMenubarAndToolbar (bool PackWidgets = true);
    void add_stock_item(const char *name[], Glib::ustring id, Glib::ustring label);
    void plotLines 
      (vector < vector <LinePair *> > PlotLines, vector <unsigned int> PlotOrder);
    void plotLines (XgSpectrum XgData, int Index);
    void generatePlots (vector < vector <LinePair *> > PlotLines);
    vector <Coord> voigtProfile (XgLine LineIn, vector <Coord> Points);
    vector <LinePair> getLinePairs (vector <KzLine *> KzLevel, int Spec);
    vector < vector <LinePair> > getLinePairs (vector <KzLine *> KzLevel);
    void getLinePairs ();
    bool matchedXgLineExists (KzLine LineIn, double Discrimintor);
    void clearDisplayedPlots ();
    void updateKuruczCompleteness ();
    void updateKuruczBF ();
    void loadXGremlinData ();
    void updatePlottedData (bool CalcScaleFactors = true);
    int do_load_expt_spectrum ();
    void writeFileVersion (ofstream *BinOut);
    void saveExptSpectra (ofstream *BinOut);
    void saveKuruczList (ofstream *BinOut);
    void saveInterface (ofstream *BinOut);
    int readFileVersion (ifstream *BinIn);
    void loadExptSpectra (ifstream *BinIn);
    void loadKuruczList (ifstream *BinIn);
    void loadInterface (ifstream *BinIn, int FileVersion);
    void refreshKuruczList ();
    void refreshSpectraList ();
    void projectHasChanged (bool Changed);
    void addToSpectraList (XgSpectrum NewSpectrum, int Index, bool Ref, bool Select);
    void addNewLines (XgSpectrum *Spectrum, vector <XgLine> NewLines);
    void updateKuruczList (KzList LineList);
    void updateXGremlinList (vector < vector <LinePair *> > OrderedPairs, 
      vector <string> SpectrumLabels, vector <unsigned int> SpectrumOrder);
    vector <DataBF> calculateBranchingFractions (vector < vector <LinePair *> > OrderedPairs, 
      vector <string> SpectrumLabels, vector <unsigned int> SpectrumOrder);
    void updateBranchingFractions (vector < vector <LinePair *> > OrderedPairs, 
      vector <string> SpectrumLabels, vector <unsigned int> SpectrumOrder);
    vector <RatioAndError> updateComparisonList (vector < vector <LinePair *> > 
      OrderedPairs, vector <string> SpectrumLabels, vector <unsigned int> SpectrumOrder);
    void saveProject (string Filename) throw (Error);
    RatioAndError compareLinkedSpectra (unsigned int a, unsigned int b, 
      Gtk::TreeModel::Row parentRow) throw (int);
    RatioAndError getBestScalingFactor (unsigned int Start, 
      unsigned int End, vector <RatioAndError> ScalingFactors) throw (int);
    
    void lifetimeValidatedOnCellData(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
    void lifetimeErrorValidatedOnCellData(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
    void lifetimeEditStarted (Gtk::CellEditable* cell_editable, const Glib::ustring& path);
    void lifetimeEdited (const Glib::ustring& path_string, const Glib::ustring& new_text);
    void lifetimeErrorEdited (const Glib::ustring& path_string, const Glib::ustring& new_text);
    
    // Error Handlers
    void display_error (Error *Err, string StatusMsg = "");
    
  protected:
    // GTK signal handlers
    void on_file_new ();
    void on_file_open ();
    void on_file_save ();
    void on_file_save_as ();
    void on_file_save_level ();
    void on_file_export_project ();
    void on_file_print_level ();
    void on_file_quit () { hide (); }
    void on_data_load_kurucz ();
    void on_data_save_kurucz ();
    void on_data_load_expt_spectrum ();
    void on_data_load_line_list ();
    void on_data_attach_standard_lamp_spectrum ();
    void on_data_attach_standard_lamp_radiance ();
    void on_data_remove_spectrum ();
    void on_data_output_results ();
    void on_tools_options ();
    void on_help_about ();
    void scaleGraphs ();
    void on_click_treeSpectra (GdkEventButton* event);
    void on_click_level_list (GdkEventButton* event);
    void on_click_level_list_bf (GdkEventButton* event);
    void on_click_treeDataXGr ();
    void on_click_plot (bool Selected);
    void row_callback_treeDataXGr (const Gtk::TreeModel::iterator& iter);
    void on_popup_remove_linelist ();
    void on_popup_remove_radiance ();
    void on_popup_remove_standard_lamp_spectrum ();
    void on_popup_link_spectrum ();
    void on_popup_remove_level ();
    void on_popup_export_linelist ();
    void on_popup_enable_line ();
    void on_popup_disable_line (bool Disable);
    void on_popup_hide_line ();
    void ref_spectrum_toggled (const Glib::ustring& path);
    bool on_delete_event (GdkEventAny* event);
    void do_link_spectrum (GdkEventButton* event);
    void abort_link_spectrum (GdkEventButton* event);

    // Model for treeLevelsBF, which contains all the level specific information
    // for branching fraction work. The bg_colour property allows the EW columns
    // to be coloured depending on whether or not all the loaded XGremlin 
    // spectra are intensity calibrated.
    //  
    //  Red background : None of the loaded spectra have a response function
    //  Yellow background : Some of the loaded spectra have a response function
    //  Green background : All of the loaded spectra have a response function
    //
    class ColumnsLevelsBF : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<int> index, branches;
        Gtk::TreeModelColumn<string> config;
        Gtk::TreeModelColumn<double> a_total, a_missing,
          ew_total, ew_norm, eupper, jupper;
        Gtk::TreeModelColumn<Gdk::Color> bg_colour;
        ColumnsLevelsBF(){add (index); add (config); add (eupper); add (jupper);
          add (a_total); add (branches); add (a_missing); 
          add (ew_total); add (ew_norm); add (bg_colour); }
    };
    
    // Model for treeDataKur, which shows the Kurucz lines for the currently
    // selected upper level. Additional colour properties are used to colour
    // lines depending on whether they are important and whether they are seen
    // in the loaded XGremlin spectra.
    //
    //  White background, grey text : Line not seen but is unimportant
    //  White background, black text : Line is seen in the loaded XGremlin data
    //  Red background, black text : Line is important but is not seen
    //
    class KuruczColumns : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<string> configlower, configupper;
        Gtk::TreeModelColumn<double> 
          sigma, wavelength, loggf, bf, elower, jlower, eupper, jupper;
        Gtk::TreeModelColumn<LineData *> profile;
        Gtk::TreeModelColumn<Gdk::Color> colour, bg_colour;
        KuruczColumns () { add (profile); add (wavelength); add (loggf); 
          add (bf); add (elower); add (jlower); add (configlower); add (eupper);
          add (jupper); add (configupper); add (colour); add (bg_colour); 
          add (sigma); }
    };
    
    // Model for treeDataXGr, which shows the XGremlin lines for the currently
    // selected upper level. The bg_colour property allows the Eq. Width column
    // to be coloured depending on whether or not the spectrum is intensity
    // calibrated.
    //
    //  Red background  : No response function associated with the spectrum
    //  Green background : The spectrum has an associated response function
    //
    class ColumnsDataXGremlin : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<int> index;
        Gtk::TreeModelColumn<double> wavenumber, peak, width, dmp, eqwidth,
          epstot, epsevn, epsodd, epsran;
        Gtk::TreeModelColumn<string> spectrum, id;
        Gtk::TreeModelColumn<LineData *> profile;
        Gtk::TreeModelColumn<Gdk::Color> bg_colour, eq_width_colour;
        ColumnsDataXGremlin () { add (spectrum); add (index); add (wavenumber);
          add (peak); add (width); add (dmp); add (eqwidth); add (epstot); 
          add (epsevn); add (epsodd); add (epsran); add (id); add (profile); 
          add (bg_colour); add (eq_width_colour); }
    };
    
    // Model for treeDataBF, which shows branching fraction and log(gf) data for
    // the lines in the currently selected upper level. The bg_colour property 
    // allows the BF and log(gf) columns to be coloured depending on whether or
    // not the experimental spectrum is intensity calibrated.
    //
    //  Red background  : No response function associated with the spectrum
    //  Green background : The spectrum has an associated response function
    //
    class ColumnsDataBF : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<int> index;
        Gtk::TreeModelColumn<double> wavenumber, eqwidth, err_line, err_cal,
          err_trans, err_total, err_eqwidth, br_frac, err_br_frac, a, err_a,
          loggf, dex;
        Gtk::TreeModelColumn<string> spectrum;
        Gtk::TreeModelColumn<LineData *> profile;
        Gtk::TreeModelColumn<Gdk::Color> bg_colour, eq_width_colour, err_cal_colour;
        ColumnsDataBF () { add (spectrum); add (index); add (wavenumber);
          add (err_line); add (err_cal); add (err_trans); add (err_total); 
          add (err_eqwidth); add (err_br_frac); add (eqwidth); add (br_frac); 
          add (loggf); add (profile); add (bg_colour); add (err_cal_colour); 
          add (a); add (err_a); add (dex); add (eq_width_colour); }
    };
    
    // Model for treeDataComp, which contains information to compare all the
    // currently loaded XGremlin spectra.
    //
    class ColumnsDataCompare : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<string> wavenumber, ref, comparison, ratio;
        Gtk::TreeModelColumn<Gdk::Color> bg_colour;
        ColumnsDataCompare () { add (ref); add (comparison); add (wavenumber);
          add (ratio); }
    };
    
    // Model for treeSpectra, which lists all the currently loaded XGremlin 
    // spectra
    //
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<string> name, note;
        Gtk::TreeModelColumn<int> emin, emax, index, line_index;
        Gtk::TreeModelColumn<bool> ref;
        Gtk::TreeModelColumn<string> label;
        Gtk::TreeModelColumn<Gdk::Color> bg_colour;
        ModelColumns() { add (label); add (ref); add (name); add (index); 
          add (line_index); add (emin); add (emax); add (note); add (bg_colour); }
    };
    
    // Implementations of the above treeModel classes
    KuruczColumns targetsCols;
    LevelColumns levelCols;
    ColumnsLevelsBF levelColsBF;
    ColumnsDataXGremlin colsDataXGr;
    ColumnsDataCompare colsDataComp;
    ColumnsDataBF colsDataBF;
    ModelColumns m_Columns;
    
  public:

    AnalyserWindow ();
    ~AnalyserWindow ();
    
    void newProject ();
    void fileOpen (string Filename);
};

#endif // LINE_ANALYSER_WINDOW
