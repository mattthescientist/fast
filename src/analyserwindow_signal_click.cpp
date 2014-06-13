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
// AnalyserWindow class (analyserwindow_signal_click.cpp)
//==============================================================================
// This file contains signal handlers related to mouse clicks events.

//------------------------------------------------------------------------------
// on_click_treeSpectra (GdkEventButton*) : Processes right-click events on the
// list of experimental spectra and then displays a suitable popup menu with
// spectrum handling options.
//
void AnalyserWindow::on_click_treeSpectra (GdkEventButton* event)
{
  if((event->type == GDK_BUTTON_PRESS) && 
    (event->button == 1 || event->button == 3)) {
  
    Gtk::TreePath path;

    // First, force the clicked tree item to be selected.
    {
      Gtk::TreeViewColumn *temp_column;
      int temp_x, temp_y;
      if (treeSpectra.get_path_at_pos(
        (int)event->x, (int)event->y, path, temp_column, temp_x, temp_y)) {
        treeSpectra.set_cursor(path);
      } else {
        if (event->button == 3) {
          menuInitialPopup.popup(event->button, event->time);
          return;
        }
      }
    }

    // Now obtain an iterator for the active selection and check to see if it
    // is a root node (and thus a spectrum) or a child (and thus a line list).    
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeSpectra.get_selection();
    if(refSelection) {
      Gtk::TreeModel::iterator iter = refSelection->get_selected();
      Gtk::TreeModel::iterator parent;
      if(iter) {
        path = treeSpectra.get_model()->get_path(iter);
        path.up ();
        
        // Try finding a parent node. If successful, the selected item must be a
        // line list. If unsuccessful, it must be a spectrum. Display the 
        // correct popup menu.
        if (path.up()) {
          if (event->button == 3) {
            if ((*iter)[m_Columns.line_index] == -1) {
              menuRadiancePopup.popup(event->button, event->time);
            } else if ((*iter)[m_Columns.line_index] == -2) {
              menuStdLampPopup.popup(event->button, event->time);
            } else {
              menuLinelistPopup.popup(event->button, event->time);
            }
          } else {
            if ((*iter)[m_Columns.line_index] != -1 
              && (*iter)[m_Columns.line_index] != -2) {
              plotLines (ExptSpectra [(*iter)[m_Columns.index]], 
                (*iter)[m_Columns.line_index]);
            } else { 
              clearDisplayedPlots ();
            }
          }
        } else {
          if (event->button == 3) {
            menuSpectraPopup.popup(event->button, event->time);
          }
        }
      }
    }
  }
}


//------------------------------------------------------------------------------
// on_click_treeDataXGr () : When a selection is made in the list of XGremlin 
// lines for the current level, these two functions are triggered to select the
// corresponding plots.
//
void AnalyserWindow::on_click_treeDataXGr () {
  // Check the current selection is valid. Only proceed if it is.
  Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeDataXGr.get_selection();
  if (treeSelection) {
    for (unsigned int i = 0; i < LineBoxes.size (); i ++) {
      for (unsigned int j = 0; j < LineBoxes[i].size (); j ++) {
        LineBoxes[i][j] -> selected (false);
      }
    }
    treeSelection->selected_foreach_iter(
      sigc::mem_fun(*this, &AnalyserWindow::row_callback_treeDataXGr));
  }
}

void AnalyserWindow::row_callback_treeDataXGr (const Gtk::TreeModel::iterator& iter) {
  if (iter) {
    LineData *SelectedLine = (*iter)[colsDataXGr.profile];
    SelectedLine -> selected (true);
  }
}


//------------------------------------------------------------------------------
// on_click_level_list () : When a Kurucz upper level is selected in treeLevels,
// this function is called to begin the process of updating the displayed line
// plots. If the right moust button was pressed, a popup context menu is shown
// with options to manage the Kurucz data.
//
void AnalyserWindow::on_click_level_list (GdkEventButton* event) {
  Gtk::TreePath path;
  if((event->type == GDK_BUTTON_PRESS) && 
    (event->button == 1 || event->button == 3)) {

    // First, force the clicked tree item to be selected.
    {
      Gtk::TreeViewColumn *temp_column;
      int temp_x, temp_y;
      if (treeLevels.get_path_at_pos(
        (int)event->x, (int)event->y, path, temp_column, temp_x, temp_y)) {
        treeLevels.set_cursor(path);
        scrollLevelsBF.set_vadjustment (scrollLevels.get_vadjustment ());
        if (treeLevelsBF.get_path_at_pos(
          (int)event->x, (int)event->y, path, temp_column, temp_x, temp_y)) {
          treeLevelsBF.set_cursor(path);
        }
      } else {
        // If no item could be selected AND the right moust button was pressed,
        // show the initial Kurucz context menu.
        if (event->button == 3) {
          menuLevelInitPopup.popup(event->button, event->time);
          return;

        // No item is selected. The right mouse btn wasn't used, so just return.
        } else {
          return;
        }
      }
    }

    // Then refresh the plotted data so as to display the newly selected level.
    updatePlottedData ();

    // Finally, display the level pop-up menu if the right button was pressed.
    if (event->button == 3) {
      menuLevelPopup.popup(event->button, event->time);
      return;
    }
  }
}


//------------------------------------------------------------------------------
// on_click_level_list_bf () : When a Kurucz upper level is selected in 
// treeLevels, this function is called to begin the process of updating the 
// displayed line plots. If the right mouse button was pressed, a popup context
// menu is shown with options to manage the Kurucz data.
//
void AnalyserWindow::on_click_level_list_bf (GdkEventButton* event) {
  Gtk::TreePath path;
  if((event->type == GDK_BUTTON_PRESS) && 
    (event->button == 1 || event->button == 3)) {

    // First, force the clicked tree item to be selected.
    {
      Gtk::TreeViewColumn *temp_column;
      int temp_x, temp_y;
      if (treeLevels.get_path_at_pos(
        (int)event->x, (int)event->y, path, temp_column, temp_x, temp_y)) {
        treeLevels.set_cursor(path);
        scrollLevels.set_vadjustment (scrollLevelsBF.get_vadjustment ());
        if (treeLevelsBF.get_path_at_pos(
          (int)event->x, (int)event->y, path, temp_column, temp_x, temp_y)) {
          treeLevelsBF.set_cursor(path);
        }
      } else {
        // If no item could be selected AND the right mouse button was pressed,
        // show the initial Kurucz context menu.
        if (event->button == 3) {
          menuLevelInitPopup.popup(event->button, event->time);
          return;

        // No item is selected. The right mouse btn wasn't used, so just return.
        } else {
          return;
        }
      }
    }

    // Then refresh the plotted data so as to display the newly selected level.
    updatePlottedData ();

    // Finally, display the level pop-up menu if the right button was pressed.
    if (event->button == 3) {
      menuLevelPopup.popup(event->button, event->time);
      return;
    }
  }
}


//------------------------------------------------------------------------------
// on_click_plot () : Updates branching fraction data when a new plot is either
// selected or unselected.
//
void AnalyserWindow::on_click_plot (bool Selected) {

	// Get the index of the currently selected upper level
	Glib::RefPtr<Gtk::TreeSelection> treeSelection = treeLevelsBF.get_selection();
    if (treeSelection) {
	  Gtk::TreeModel::iterator iter = treeLevelsBF.get_selection()->get_selected();
	  if (iter) {
	    int Level = (*iter)[levelCols.index];
	    bool FoundLine = false;

		// Look at each line in turn to identify the plot that the user clicked on.
	    // Check to make sure that this line is only selected in that spectrum.
	    // De-select the line in all other spectra.
		for (unsigned int j = 0; j < LevelLines[Level][0].size (); j ++) {
			for (unsigned int i = 0; i < LevelLines[Level].size (); i ++) {
				if (LevelLines[Level][i][j].plot->changed()) {
					LevelLines[Level][i][j].plot->clearChanged();
					for (unsigned int k = 0; k < LevelLines[Level].size (); k ++) {
						if (LevelLines[Level][k][j].plot->selected() && k != i) {
							LevelLines[Level][k][j].plot->selected(false);
						}
					}
					FoundLine = true;
					break;
				}
			}
			if (FoundLine) break;
		}
	  }
    }

    // Now update the plotted data and branching fraction information
    updatePlottedData (false);
    updateKuruczCompleteness ();
    projectHasChanged (true);
}


