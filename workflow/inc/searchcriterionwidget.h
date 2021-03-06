/***************************************************************************
 *  searchcriterionwidget - Search engine criterion definition widget
 *
 *  Jul 2 21:10:56 2009
 *  Copyright  2007  Christophe Seyve
 *  Email cseyve@free.fr
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef SEARCHCRITERIONWIDGET_H
#define SEARCHCRITERIONWIDGET_H

#include <QWidget>

namespace Ui {
    class SearchCriterionWidget;
}

class SearchCriterionWidget : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(SearchCriterionWidget)
public:
    explicit SearchCriterionWidget(QWidget *parent = 0);
    virtual ~SearchCriterionWidget();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::SearchCriterionWidget *m_ui;
};

#endif // SEARCHCRITERIONWIDGET_H
