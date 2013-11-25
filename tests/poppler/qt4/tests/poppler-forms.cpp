#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <iostream>

#include <poppler-qt4.h>
#include <poppler-form.h>

std::ostream& operator<< (std::ostream &out, Poppler::FormField::FormType type)
{
    switch (type) {
        case Poppler::FormField::FormButton:    out << "Button";    break;
        case Poppler::FormField::FormText:      out << "Text";      break;
        case Poppler::FormField::FormChoice:    out << "Choice";    break;
        case Poppler::FormField::FormSignature: out << "Signature"; break;
    }
    return out;
}

std::ostream& operator<< (std::ostream &out, Poppler::FormFieldButton::ButtonType type)
{
    switch (type) {
        case Poppler::FormFieldButton::Push:        out << "Push";      break;
        case Poppler::FormFieldButton::CheckBox:    out << "CheckBox";  break;
        case Poppler::FormFieldButton::Radio:       out << "Radio";     break;
    }
    return out;
}

std::ostream& operator<< (std::ostream &out, Poppler::FormFieldText::TextType type)
{
    switch (type) {
        case Poppler::FormFieldText::Normal:        out << "Normal";        break;
        case Poppler::FormFieldText::Multiline:     out << "Multiline";     break;
        case Poppler::FormFieldText::FileSelect:    out << "FileSelect";    break;
    }
    return out;
}

std::ostream& operator<< (std::ostream &out, Poppler::FormFieldChoice::ChoiceType type)
{
    switch (type) {
        case Poppler::FormFieldChoice::ComboBox:      out << "ComboBox";    break;
        case Poppler::FormFieldChoice::ListBox:       out << "ListBox";     break;
    }
    return out;
}

std::ostream& operator<< (std::ostream &out, Qt::Alignment alignment)
{
    switch (alignment) {
        case Qt::AlignLeft:     out << "Left";      break;
        case Qt::AlignRight:    out << "Right";     break;
        case Qt::AlignHCenter:  out << "HCenter";   break;
        case Qt::AlignJustify:  out << "Justify";   break;
        case Qt::AlignTop:      out << "Top";       break;
        case Qt::AlignBottom:   out << "Bottom";    break;
        case Qt::AlignVCenter:  out << "VCenter";   break;
        case Qt::AlignCenter:   out << "Center";    break;
        case Qt::AlignAbsolute: out << "Absolute";  break;
    }
    return out;
}

std::ostream& operator<< (std::ostream &out, const QString &string)
{
    out << string.toUtf8().constData();
    return out;
}

std::ostream& operator<< (std::ostream &out, const QRectF &rect)
{
    out << QString("top: %1 left: %2 width: %3 height: %4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
    return out;
}

template<typename T>
std::ostream& operator<< (std::ostream &out, const QList<T> &elems)
{
    bool isFirst = true;
    for (int i = 0; i < elems.count(); ++i) {
        if (!isFirst)
            out << " ";
        out << elems[i];
        isFirst = false;
    }
    return out;
}

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );

    if (!( argc == 2 ))
    {
        qWarning() << "usage: poppler-forms filename";
        exit(1);
    }

    Poppler::Document *doc = Poppler::Document::load(argv[1]);
    if (!doc)
    {
        qWarning() << "doc not loaded";
        exit(1);
    }

    std::cout << "Forms for file " << argv[1] << std::endl;
    for (int i = 0; i < doc->numPages(); ++i) {
        Poppler::Page *page = doc->page(i);
        if (page) {
            QList<Poppler::FormField*> forms = page->formFields();
            std::cout << "\tPage " << i + 1 << std::endl;
            foreach( const Poppler::FormField *form, forms ) {
                std::cout << "\t\tForm" << std::endl;
                std::cout << "\t\t\tType: " << form->type() << std::endl;
                std::cout << "\t\t\tRect: " << form->rect() << std::endl;
                std::cout << "\t\t\tID: " << form->id() << std::endl;
                std::cout << "\t\t\tName: " << form->name() << std::endl;
                std::cout << "\t\t\tFullyQualifiedName: " << form->fullyQualifiedName() << std::endl;
                std::cout << "\t\t\tUIName: " << form->uiName() << std::endl;
                std::cout << "\t\t\tReadOnly: " << form->isReadOnly() << std::endl;
                std::cout << "\t\t\tVisible: " << form->isVisible() << std::endl;
                switch (form->type()) {
                    case Poppler::FormField::FormButton: {
                        const Poppler::FormFieldButton *buttonForm = static_cast<const Poppler::FormFieldButton *>(form);
                        std::cout << "\t\t\tButtonType: " << buttonForm->buttonType() << std::endl;
                        std::cout << "\t\t\tCaption: " << buttonForm->caption() << std::endl;
                        std::cout << "\t\t\tState: " << buttonForm->state() << std::endl;
                        std::cout << "\t\t\tSiblings: " << buttonForm->siblings() << std::endl;
                    }
                    break;
                    
                    case Poppler::FormField::FormText: {
                        const Poppler::FormFieldText *textForm = static_cast<const Poppler::FormFieldText *>(form);
                        std::cout << "\t\t\tTextType: " << textForm->textType() << std::endl;
                        std::cout << "\t\t\tText: " << textForm->text() << std::endl;
                        std::cout << "\t\t\tIsPassword: " << textForm->isPassword() << std::endl;
                        std::cout << "\t\t\tIsRichText: " << textForm->isRichText() << std::endl;
                        std::cout << "\t\t\tMaximumLength: " << textForm->maximumLength() << std::endl;
                        std::cout << "\t\t\tTextAlignment: " << textForm->textAlignment() << std::endl;
                        std::cout << "\t\t\tCanBeSpellChecked: " << textForm->canBeSpellChecked() << std::endl;
                    }
                    break;

                    case Poppler::FormField::FormChoice: {
                        const Poppler::FormFieldChoice *choiceForm = static_cast<const Poppler::FormFieldChoice *>(form);
                        std::cout << "\t\t\tChoiceType: " << choiceForm->choiceType() << std::endl;
                        std::cout << "\t\t\tChoices: " << choiceForm->choices() << std::endl;
                        std::cout << "\t\t\tIsEditable: " << choiceForm->isEditable() << std::endl;
                        std::cout << "\t\t\tIsMultiSelect: " << choiceForm->multiSelect() << std::endl;
                        std::cout << "\t\t\tCurrentChoices: " << choiceForm->currentChoices() << std::endl;
                        std::cout << "\t\t\tEditChoice: " << choiceForm->editChoice() << std::endl;
                        std::cout << "\t\t\tTextAlignment: " << choiceForm->textAlignment() << std::endl;
                        std::cout << "\t\t\tCanBeSpellChecked: " << choiceForm->canBeSpellChecked() << std::endl;
                    }
                    break;

                    case Poppler::FormField::FormSignature:
                    break;
                }
            }
            qDeleteAll(forms);
        }
    }
    delete doc;
}
