/* poppler-form.h: qt4 interface to poppler
 * Copyright (C) 2007-2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, Albert Astals Cid <aacid@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _POPPLER_QT4_FORM_H_
#define _POPPLER_QT4_FORM_H_

#include <QtCore/QRectF>
#include <QtCore/QStringList>
#include "poppler-export.h"

class Page;
class FormWidget;
class FormWidgetButton;
class FormWidgetText;
class FormWidgetChoice;

namespace Poppler {

    class DocumentData;
    class Link;

    class FormFieldData;
    /**
      The base class representing a form field.

      \since 0.6
     */
    class POPPLER_QT4_EXPORT FormField {
    public:

	/**
	   The different types of form field.
	*/
	enum FormType {
	    FormButton,    ///< A button field. See \ref Poppler::FormFieldButton::ButtonType "ButtonType"
	    FormText,      ///< A text field. See \ref Poppler::FormFieldText::TextType "TextType"
	    FormChoice,    ///< A single choice field. See \ref Poppler::FormFieldChoice::ChoiceType "ChoiceType"
	    FormSignature  ///< A signature field.
	};

	virtual ~FormField();

	/**
	  The type of the field.
	 */
	virtual FormType type() const = 0;

	/**
	   \return The size of the field, in normalized coordinates, i.e.
	   [0..1] with regard to the dimensions (cropbox) of the page
	*/
	QRectF rect() const;

	/**
	  The ID of the field.
	 */
	int id() const;

	/**
	  The internal name of the field.
	 */
	QString name() const;

	/**
	  The name of the field to be used in user interface (eg messages to
	  the user).
	 */
	QString uiName() const;

	/**
	  Whether this form field is read-only.
	 */
	bool isReadOnly() const;

	/**
	  Whether this form field is visible.
	 */
	bool isVisible() const;

	/**
	  The activation action of this form field.

	  \note It may be null.
	 */
	Link* activationAction() const;

    protected:
	/// \cond PRIVATE
	FormField(FormFieldData &dd);

	FormFieldData *m_formData;
	/// \endcond

    private:
	Q_DISABLE_COPY(FormField)
    };

    /**
      A form field that represents a "button".

      \since 0.8
     */
    class POPPLER_QT4_EXPORT FormFieldButton : public FormField {
    public:

	/**
	 * The types of button field.
	 */
	enum ButtonType
	{
	    Push,          ///< A simple push button.
	    CheckBox,      ///< A check box.
	    Radio          ///< A radio button.
	};

	/// \cond PRIVATE
	FormFieldButton(DocumentData *doc, ::Page *p, ::FormWidgetButton *w);
	/// \endcond
	virtual ~FormFieldButton();

	virtual FormType type() const;

	/**
	  The particular type of the button field.
	 */
	ButtonType buttonType() const;

	/**
	 * The caption to be used for the button.
	 */
	QString caption() const;

	/**
	  The state of the button.
	 */
	bool state() const;

	/**
	  Sets the state of the button to the new \p state .
	 */
	void setState( bool state );

	/**
	  The list with the IDs of siblings (ie, buttons belonging to the same
	  group as the current one.

	  Valid only for \ref Radio buttons, an empty list otherwise.
	 */
	QList<int> siblings() const;

    private:
	Q_DISABLE_COPY(FormFieldButton)
    };

    /**
      A form field that represents a text input.

      \since 0.6
     */
    class POPPLER_QT4_EXPORT FormFieldText : public FormField {
    public:

	/**
	   The particular type of this text field.
	*/
	enum TextType {
	    Normal,        ///< A simple singleline text field.
	    Multiline,     ///< A multiline text field.
	    FileSelect     ///< An input field to select the path of a file on disk.
	};

	/// \cond PRIVATE
	FormFieldText(DocumentData *doc, ::Page *p, ::FormWidgetText *w);
	/// \endcond
	virtual ~FormFieldText();

	virtual FormType type() const;

	/**
	  The text type of the text field.
	 */
	TextType textType() const;

	/**
	  The text associated with the text field.
	 */
	QString text() const;

	/**
	  Sets the text associated with the text field to the specified
	  \p text.
	 */
	void setText( const QString& text );

	/**
	  Whether this text field is a password input, eg its text \b must be
	  replaced with asterisks.

	  Always false for \ref FileSelect text fields.
	 */
	bool isPassword() const;

	/**
	  Whether this text field should allow rich text.
	 */
	bool isRichText() const;

	/**
	  The maximum length for the text of this field, or -1 if not set.
	 */
	int maximumLength() const;

	/**
	  The horizontal alignment for the text of this text field.
	 */
	Qt::Alignment textAlignment() const;

	/**
	  Whether the text inserted manually in the field (where possible)
	  can be spell-checked.
	 */
	bool canBeSpellChecked() const;

    private:
	Q_DISABLE_COPY(FormFieldText)
    };

    /**
      A form field that represents a choice field.

      \since 0.6
     */
    class POPPLER_QT4_EXPORT FormFieldChoice : public FormField {
    public:

	/**
	   The particular type of this choice field.
	*/
	enum ChoiceType {
	    ComboBox,     ///< A simple singleline text field.
	    ListBox       ///< A multiline text field.
	};

	/// \cond PRIVATE
	FormFieldChoice(DocumentData *doc, ::Page *p, ::FormWidgetChoice *w);
	/// \endcond
	virtual ~FormFieldChoice();

	virtual FormType type() const;

	/**
	  The choice type of the choice field.
	 */
	ChoiceType choiceType() const;

	/**
	  The possible choices of the choice field.
	 */
	QStringList choices() const;

	/**
	  Whether this FormFieldChoice::ComboBox is editable, i.e. the user
	  can type in a custom value.

	  Always false for the other types of choices.
	 */
	bool isEditable() const;

	/**
	  Whether more than one choice of this FormFieldChoice::ListBox
	  can be selected at the same time.

	  Always false for the other types of choices.
	 */
	bool multiSelect() const;

	/**
	  The currently selected choices.
	 */
	QList<int> currentChoices() const;

	/**
	  Sets the selected choices to \p choice.
	 */
	void setCurrentChoices( const QList<int> &choice );

	/**
	  The horizontal alignment for the text of this text field.
	 */
	Qt::Alignment textAlignment() const;

	/**
	  Whether the text inserted manually in the field (where possible)
	  can be spell-checked.

          Returns false if the field is not an editable text field.
	 */
	bool canBeSpellChecked() const;

    private:
	Q_DISABLE_COPY(FormFieldChoice)
    };

}

#endif
