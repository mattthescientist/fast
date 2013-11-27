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
// OutputWindow class (outputwindow.h)
//==============================================================================
// Displays a window window containing the necessary functionality to output
// FAST results to Text/CSV or LaTeX file formats
//
#ifndef LINE_ANALYSER_OUTPUT_WINDOW
#define LINE_ANALYSER_OUTPUT_WINDOW

// Include the GTK+ environment from GTKmm header files
#include <gtkmm/window.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/stock.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/table.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/image.h>
#include <gtkmm/settings.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/comboboxtext.h>
#include <string>

#include "TypeDefs.h"
#include "ErrDefs.h"
#include "xgline.h"
#include "kzline.h"
#include "linedata.h"

#define OUTPUT_BF_LIST 0x01
#define OUTPUT_TARGET_LIST 0x02
#define OUTPUT_FITTED_LIST 0x04

using namespace::std;

typedef struct output_field {
  string Name;                        // Field name to be displayed
  unsigned int ResultIndex;           // Index of this element in the 'Results' vector
  bool Active;                        // Set to true when this field is included in move operations
  bool Selected;                      // Set to true when this field is selected for output
  vector < vector <string> > Value;   // A copy of the field value, obtained from AnalyserWindow
  unsigned short int List;
} OutputField;

#define NUM_OUTPUTWINDOW_TYPES 3
const string OUTPUTWINDOW_TYPES [NUM_OUTPUTWINDOW_TYPES] = 
  {"Text / CSV", "LaTeX table", "AASTeX deluxetable"};

class OutputWindow : public Gtk::Window {
  private:
    vector <vector <DataBF> > Results;
    vector <vector <XgLine> > Fits;
    vector <vector <KzLine> > Targets;
    vector <OutputField> ResultStrings;
    vector <OutputField *> AvailableFields;
    vector <OutputField *> SelectedFields;
    bool DisplaySet;

    // GTKmm widgets
    Gtk::ScrolledWindow Scroll;
    Gtk::VBox BaseVBox;
    Gtk::HBox BaseHBox;
    
    // Widgets for listing the data selection options
    Gtk::VBox LeftBox;
    Gtk::Frame FrameListFiles;
    Gtk::VBox BoxListFiles;
    Gtk::CheckButton ButtonTargets;
    Gtk::CheckButton ButtonProfiles;
    Gtk::CheckButton ButtonBFs;
    Gtk::ComboBoxText ComboOutputType;
    Gtk::Frame FrameListDelimiters;
    Gtk::VBox BoxListDelimiters;
    Gtk::RadioButton ButtonDelimitComma;
    Gtk::RadioButton ButtonDelimitSpace;
    Gtk::RadioButton ButtonDelimitTab;
    Gtk::RadioButton ButtonDelimitOther;
    Gtk::Entry EntryDelimitOther;    

    Gtk::Frame FrameFieldSelection;
    Gtk::HBox BoxFieldSelection;
    Gtk::ScrolledWindow ScrollAvailable;
    Gtk::TreeView TreeAvailableFields;
    Gtk::VBox BoxFieldSelectButtons;
    Gtk::VBox BoxFieldSelectButtons2;
    Gtk::ScrolledWindow ScrollSelected;
    Gtk::TreeView TreeSelectedFields;
    Gtk::VBox BoxFieldMoveButtons;
    Gtk::VBox BoxFieldMoveButtons2;
    Gtk::Button ButtonAdd;
    Gtk::Button ButtonAddAll;
    Gtk::Button ButtonRemove;
    Gtk::Button ButtonRemoveAll;
    Gtk::Button ButtonMoveUp;
    Gtk::Button ButtonMoveDown;
    
    Gtk::Frame FrameFileSelection;
    Gtk::HBox BoxFileSelection;
    Gtk::Entry EntryFileSelection;
    Gtk::Button ButtonBrowse;

    Gtk::HBox BoxSaveCancel;
    Gtk::Button ButtonSave;
    Gtk::Button ButtonCancel;
    Glib::RefPtr<Gtk::ListStore> modelFieldsAvailable;
    Glib::RefPtr<Gtk::ListStore> modelFieldsSelected;
    
    void on_button_save ();
    void on_button_cancel () { hide (); }
    void on_button_add_all ();
    void on_button_add ();
    void on_button_remove ();
    void on_button_remove_all ();
    void on_button_move_up ();
    void on_button_move_down ();
    void on_button_lists ();
    void on_button_browse ();
    void on_button_delimit_other ();
    void on_change_combo_output ();
    
    void row_callback_button_add (const Gtk::TreeModel::iterator& iter);
    void row_callback_button_remove (const Gtk::TreeModel::iterator& iter);
    void row_callback_button_move (const Gtk::TreeModel::iterator& iter);
    
    void create_image_buttons ();
    void set_button_signals ();
    void set_result_strings ();
    void set_display_fields ();
    void display_fields ();
    
    void writeCSV (string Filename) throw (Error);
    void writeLaTeX (string Filename) throw (Error);
    void writeAASTeX (string Filename) throw (Error);
    string getLaTeXRows ();
    void confirm_file_overwrite (string Filename) throw (Error);
    
    class ColumnsFields : public Gtk::TreeModel::ColumnRecord {
      public:
        Gtk::TreeModelColumn<string> Name;
        Gtk::TreeModelColumn<unsigned int> ResultIndex, ListIndex;
        ColumnsFields() { add (Name); add(ListIndex); add (ResultIndex); }
    };
    ColumnsFields fieldsCols;
    
  public:

    OutputWindow ();
    ~OutputWindow () { /* Does nothing */ }

    void set_results (vector <vector <DataBF> > ResultsIn, vector <vector <XgLine> > FitsIn, vector <vector <KzLine> >);
};

#endif // LINE_ANALYSER_OUTPUT_WINDOW
