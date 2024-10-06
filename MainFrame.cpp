#include "MainFrame.h"
#include "classes.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <wx/regex.h>


using namespace std;

MainFrame::MainFrame(const wxString& title) 
    : wxFrame(nullptr, wxID_ANY, title), 
    adminAccount("00000", "admin", "123456", "09-28-2004", "+63 909-320-5093", 1000000.0),
    bank(adminAccount) 
{  
    
    /*wxMessageBox(wxString::Format("Admin Balance: %f", adminAccount.balance), "Balance Inquiry", wxOK | wxICON_INFORMATION);
    wxMessageBox(wxString::Format("Current Account Balance: %f", bank.currentAccount.balance), "Balance Inquiry", wxOK | wxICON_INFORMATION);*/
    bank.retrieveAllAccounts();
    CreateControls();
    SetupSizers();
    BindEventHandlers();
}

void MainFrame::CreateControls()
{
    wxFont headlineFont(wxFontInfo(wxSize(0, 36)).Bold());
    wxFont mainFont(wxFontInfo(wxSize(0, 20)));

    panel = new wxPanel(this);
    panel->SetFont(mainFont);

    timer = new wxTimer(this, wxID_ANY);
    timer->Start(2000);
    
    insertCardText = new wxStaticText(panel, wxID_ANY, "Please insert card...");
    insertCardText->SetFont(headlineFont);

    // Enter PIN Code Field
    enterPincodeText = new wxStaticText(panel, wxID_ANY, "Enter PIN Code");
    enterPincodeText->SetFont(headlineFont);
    pincodeInputField = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    enterPincodeButton = new wxButton(panel, wxID_ANY, "Enter PIN Code", wxDefaultPosition, wxDefaultSize);

    // Registration Controlsz`
    registrationText = new wxStaticText(panel, wxID_ANY, "Register for an Account");
    registrationText->SetFont(headlineFont);

    
    accountNameInputField = new wxTextCtrl(panel, wxID_ANY, "Name: ", wxDefaultPosition, wxDefaultSize); 
    birthdayInputField = new wxTextCtrl(panel, wxID_ANY, "Birthday (MM-DD-YYYY): ", wxDefaultPosition, wxDefaultSize);
    contactNumberInputField = new wxTextCtrl(panel, wxID_ANY, "Contact Number(+63 000 - 000 - 0000) : ", wxDefaultPosition, wxDefaultSize);
    pinCodeInputField = new wxTextCtrl(panel, wxID_ANY, "PIN Code (000000): ", wxDefaultPosition, wxDefaultSize); 

    registerButton = new wxButton(panel, wxID_ANY, "Register");
  
    // Transaction Controls
    transactionText = new wxStaticText(panel, wxID_ANY, "Choose a Transaction");
    transactionText->SetFont(headlineFont);
    balanceInquiryButton = new wxButton(panel, wxID_ANY, "Balance Inquiry");
    withdrawButton = new wxButton(panel, wxID_ANY, "Withdraw");
    depositButton = new wxButton(panel, wxID_ANY, "Deposit");
    fundTransferButton = new wxButton(panel, wxID_ANY, "Fund Transfer");
    changePinButton = new wxButton(panel, wxID_ANY, "Change PIN Code");

    // Balance Inquiry Controls
    balanceText = new wxStaticText(panel, wxID_ANY, "Current Balance: -1");

    ShowInsertCardText(true);  
    ShowEnterPincode(false);
    ShowRegistrationControls(false);
    ShowTransactionControls(false);

    statusBar = CreateStatusBar();
    statusBar->SetDoubleBuffered(true);
}

void MainFrame::SetupSizers()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(insertCardText, wxSizerFlags().Center());

    // Add a Box Sizer for PIN Code field and button
    wxBoxSizer* inputPinSizer = new wxBoxSizer(wxHORIZONTAL);
    inputPinSizer->Add(pincodeInputField, wxSizerFlags().Proportion(1));
    inputPinSizer->AddSpacer(5);
    inputPinSizer->Add(enterPincodeButton);

    mainSizer->Add(enterPincodeText, wxSizerFlags().CenterHorizontal());
    mainSizer->Add(inputPinSizer, wxSizerFlags().Expand());
    
    // Add Registration Controls to Sizer
    mainSizer->Add(registrationText, wxSizerFlags().CenterHorizontal());
    mainSizer->Add(accountNameInputField, wxSizerFlags().Expand());
    mainSizer->Add(birthdayInputField, wxSizerFlags().Expand());
    mainSizer->Add(contactNumberInputField, wxSizerFlags().Expand());
    mainSizer->Add(pinCodeInputField, wxSizerFlags().Expand());
    mainSizer->Add(registerButton, wxSizerFlags().Expand());
    
    // Add Transaction Controls to Sizer
    mainSizer->Add(transactionText, wxSizerFlags().CenterHorizontal());
    mainSizer->Add(balanceInquiryButton, wxSizerFlags().Expand());
    mainSizer->Add(withdrawButton, wxSizerFlags().Expand());
    mainSizer->Add(depositButton, wxSizerFlags().Expand());
    mainSizer->Add(fundTransferButton, wxSizerFlags().Expand());
    mainSizer->Add(changePinButton, wxSizerFlags().Expand());

    // Add Balance Inquiry Controls to Sizer
    mainSizer->Add(balanceText, wxSizerFlags().CenterHorizontal());

    // Outer Sizer for Border 
    wxGridSizer* outerSizer = new wxGridSizer(1);
    outerSizer->Add(mainSizer, wxSizerFlags(1).Border(wxALL, 25).Expand());

    panel->SetSizer(outerSizer);
    outerSizer->SetSizeHints(this);
}

void MainFrame::BindEventHandlers()
{
    this->Bind(wxEVT_TIMER, &MainFrame::OnTimer, this);
    balanceInquiryButton->Bind(wxEVT_BUTTON, &MainFrame::OnBalanceInquiryButtonClicked, this);
    registerButton->Bind(wxEVT_BUTTON, &MainFrame::OnRegisterButtonClicked, this);

    // Bind focus and unfocus events for the text controls
    accountNameInputField->Bind(wxEVT_SET_FOCUS, &MainFrame::OnFocus, this);
    accountNameInputField->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnUnfocus, this);

    birthdayInputField->Bind(wxEVT_SET_FOCUS, &MainFrame::OnFocus, this);
    birthdayInputField->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnUnfocus, this);

    contactNumberInputField->Bind(wxEVT_SET_FOCUS, &MainFrame::OnFocus, this);
    contactNumberInputField->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnUnfocus, this);

    pinCodeInputField->Bind(wxEVT_SET_FOCUS, &MainFrame::OnFocus, this);
    pinCodeInputField->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnUnfocus, this);

    this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnWindowClose, this);
}

void MainFrame::OnTimer(wxTimerEvent& evt)
{
    if (IsFlashDriveInserted('D'))
    {
        OnFlashDriveInserted();
    }

    else
    {
        ShowInsertCardText(false);
        ShowEnterPincode(false);
        ShowRegistrationControls(true);
        ShowTransactionControls(false);
        panel->Layout();
    }
}

void MainFrame::OnFlashDriveInserted()
{
    if (bank.isCardRegistered())
    {
        ShowEnterPincode(false);
        ShowInsertCardText(false);
        ShowRegistrationControls(false);
        ShowTransactionControls(true);                                                              
    }
    else
    {
        ShowRegistrationControls(true);
        ShowInsertCardText(false);
        ShowEnterPincode(false);
        ShowTransactionControls(false);
    }
    panel->Layout();
}

void MainFrame::OnRegisterButtonClicked(wxCommandEvent& evt)
{
    wxString name = accountNameInputField->GetValue();
    wxString birthday = birthdayInputField->GetValue();
    wxString contactNumber = contactNumberInputField->GetValue();
    wxString pincode = pinCodeInputField->GetValue();

    // Define your placeholders
    wxString namePlaceholder = "Name: ";
    wxString birthdayPlaceholder = "Birthday (MM-DD-YYYY): ";
    wxString contactNumberPlaceholder = "Contact Number (+63 000-000-0000): ";
    wxString pinCodePlaceholder = "PIN Code (000000): ";

    // Basic validation (optional)
    if (name.IsEmpty() || name == namePlaceholder || birthday.IsEmpty() || birthday == birthdayPlaceholder || 
        contactNumber == contactNumberPlaceholder || contactNumber.IsEmpty() || pincode.IsEmpty() || pincode == pinCodePlaceholder)
    {
        wxMessageBox("Please fill in all the fields.", "Registration Error", wxOK | wxICON_ERROR);
        return;
    }

    if (name.length() > 30)
    {
        wxMessageBox("Name must not exceed 30 characters.", "Registration Error", wxOK | wxICON_ERROR);
        return;
    }
  
    if (!wxRegEx("^(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])-(\\d{4})$").Matches(birthday))
    {
        wxMessageBox("Birthday must be in the format MM-DD-YYYY.", "Registration Error", wxOK | wxICON_ERROR);
        return;
    }
    
    if (!wxRegEx("^(\\+63|0)\\d{10}$").Matches(contactNumber))
    {
        wxMessageBox("Contact Number must be in the format +630000000000 or 00000000000 (without spaces).", "Registration Error", wxOK | wxICON_ERROR);
        return;
    }
    
    if (!wxRegEx("^\\d{6}$").Matches(pincode))
    {
        wxMessageBox("PIN Code must be exactly 6 digits.", "Registration Error", wxOK | wxICON_ERROR);
        return;
    }

    string accountNumber = bank.generateAccountNumber(); // Increment and format

    // Initial balance for new accounts (0.0 for new registrations)
    double initialBalance = 0.0;

    // Create an Account object
    Account newAccount(accountNumber,
        name.ToStdString(), 
        pincode.ToStdString(), 
        birthday.ToStdString(), 
        contactNumber.ToStdString(), 
        initialBalance);

    // Add the new account to the bank
    bank.add(newAccount);

    // Check if flash drive is inserted before saving the account
    if (IsFlashDriveInserted('D'))
    {
        // Path to save the account on the flash drive
        string filepath = "D:\\account.txt";

        // Open the file on the flash drive for writing
        ofstream outFile(filepath, ios::app);  // Use append mode to prevent overwriting
        if (outFile.is_open())
        {
            outFile << newAccount.accountNumber << ","
                << newAccount.name << ","
                << newAccount.pincode << ","
                << newAccount.birthday << ","
                << newAccount.contactNumber << ","
                << newAccount.balance << endl;

            outFile.close();

            // Notify the user of the successful save
            wxMessageBox("Account Registered and saved to the flash drive!", "Success", wxOK | wxICON_INFORMATION);
        }
        else
        {
            wxMessageBox("Error: Unable to write to the flash drive.", "Error", wxOK | wxICON_ERROR);
        }
    }
    else
    {
        wxMessageBox("Flash drive not detected. Account registered, but not saved to the flash drive.", "Registration Successful", wxOK | wxICON_WARNING);
    }   

   
    // Reset the input fields
    accountNameInputField->SetValue(namePlaceholder);
    birthdayInputField->SetValue(birthdayPlaceholder);
    contactNumberInputField->SetValue(contactNumberPlaceholder);
    pinCodeInputField->SetValue(pinCodePlaceholder);
}

void MainFrame::OnWindowClose(wxCloseEvent& event)
{
    // Before closing, retrieve temporary accounts from the flash drive and add to bank
    string flashDriveFilePath = string(1, bank.getDriveLetter()) + ":\\tempAccount.txt";
    ifstream inputFile(flashDriveFilePath);

    if (inputFile.is_open())
    {
        string line;
        while (getline(inputFile, line))
        {
            stringstream stream(line);
            string accountNumber, name, pincode, birthday, contactNumber, balanceStr;
            double balance;

            getline(stream, accountNumber, ',');
            getline(stream, name, ',');
            getline(stream, pincode, ',');
            getline(stream, birthday, ',');
            getline(stream, contactNumber, ',');
            getline(stream, balanceStr, ',');

            balance = stod(balanceStr);

            // Create an Account object and add it to the bank
            Account newAccount(accountNumber, name, pincode, birthday, contactNumber, balance);
            if (!bank.accountExists(accountNumber)) 
            {
                bank.add(newAccount);
            }
        }
        inputFile.close(); 
    }
    bank.saveAllAccounts();
    //wxMessageBox("Account Registered and saved to all accounts!", "Success", wxOK | wxICON_INFORMATION);
    event.Skip();
}

void MainFrame::OnFocus(wxFocusEvent& event)
{
    wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if (textCtrl)
    {
        wxString placeholder;
        if (textCtrl == accountNameInputField)
            placeholder = "Name: ";
        else if (textCtrl == birthdayInputField)
            placeholder = "Birthday (MM-DD-YYYY): ";
        else if (textCtrl == contactNumberInputField)   
            placeholder = "Contact Number (+63 000-000-0000): ";
        else if (textCtrl == pinCodeInputField)
            placeholder = "PIN Code (000000): ";

        if (textCtrl->GetValue() == placeholder)
        {
            textCtrl->SetValue("");
        }
    }
    event.Skip();
}

void MainFrame::OnUnfocus(wxFocusEvent& event)
{
    wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    if (textCtrl)
    {
        wxString placeholder;
        if (textCtrl == accountNameInputField)
            placeholder = "Name: ";
        else if (textCtrl == birthdayInputField)
            placeholder = "Birthday (MM-DD-YYYY): ";
        else if (textCtrl == contactNumberInputField)
            placeholder = "Contact Number (+63 000-000-0000): ";    
        else if (textCtrl == pinCodeInputField)
            placeholder = "PIN Code (000000): ";

        if (textCtrl->GetValue().IsEmpty())
        {
            textCtrl->SetValue(placeholder);
        }
    }
    event.Skip();
}

bool MainFrame::IsFlashDriveInserted(char driveLetter)
{
    wstring drivePath = wstring(1, driveLetter) + L":\\";

    UINT driveType = GetDriveType(drivePath.c_str());

    // DRIVE_REMOVABLE represents removable drives like USB flash drives
    return (driveType == DRIVE_REMOVABLE);
}

void MainFrame::ShowInsertCardText(bool show)
{
    insertCardText->Show(show);
}

void MainFrame::ShowEnterPincode(bool show)
{
    enterPincodeText->Show(show);
    pincodeInputField->Show(show);
    enterPincodeButton->Show(show);
}

void MainFrame::ShowRegistrationControls(bool show)
{
    registrationText->Show(show);
    accountNameInputField->Show(show);
    birthdayInputField->Show(show);
    contactNumberInputField->Show(show);
    pinCodeInputField->Show(show);
    registerButton->Show(show);
}

void MainFrame::ShowTransactionControls(bool show)
{
    transactionText->Show(show);
    balanceInquiryButton->Show(show);
    withdrawButton->Show(show);
    depositButton->Show(show);
    fundTransferButton->Show(show);
    changePinButton->Show(show);
}

void MainFrame::OnBalanceInquiryButtonClicked(wxCommandEvent& evt)
{
    ShowTransactionControls(false);
    int balance = bank.currentAccount.balance;
 
    balanceText->SetLabel(wxString::Format("Current Balance: %d", balance));

    balanceText->Show(true);
    panel->Layout();
}

