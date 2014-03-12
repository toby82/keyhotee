#pragma once
#include <memory>
#include <unordered_map>
#include <bts/addressbook/addressbook.hpp>
#include <bts/application.hpp>

#include "ATopLevelWindowsContainer.hpp"
#include "ConnectionProcessor.hpp"

#include "AddressBook/AuthorizationItem.hpp"

#include "ch/GuiUpdateSink.hpp"
#include "ch/ModificationsChecker.hpp"

#include "qtreusable/selfsizingmainwindow.h"

#include <QList>
#include <QTreeWidget>

namespace Ui { class KeyhoteeMainWindow; }

class QTreeWidgetItem;
class QLineEdit;
class QCompleter;

class AddressBookModel;
class AuthorizationView;
class Contact;
class ContactView;
class ContactsTable;
class InboxView;
class Mailbox;
class MailboxModel;
class Mailbox;
class MenuEditControl;
class KeyhoteeMainWindow;
class TKeyhoteeApplication;

/**
 *  GUI widgets and GUI state for a contact.
 *  Not all contacts have this, only contacts "active" in GUI.
 */
class ContactGui
{
  friend KeyhoteeMainWindow;

private:
  unsigned int     _unread_msg_count;
  QTreeWidgetItem* _tree_item;
  ContactView*     _view;

public:
  ContactGui() {}
  ContactGui(QTreeWidgetItem* tree_item, ContactView* view)
    : _unread_msg_count(0), _tree_item(tree_item), _view(view) {}

  void updateTreeItemDisplay();
  void setUnreadMsgCount(unsigned int count);
  bool isChatVisible();
  void receiveChatMessage(const QString& from, const QString& msg, const QDateTime& dateTime);
private:
};

class KeyhoteeMainWindow  : public ATopLevelWindowsContainer,
                            protected IModificationsChecker,
                            protected IGuiUpdateSink
{
  Q_OBJECT
public:

  void newMailMessage();
  void newMailMessageTo(const Contact& contact);
  void addToContacts(const bts::addressbook::wallet_contact& wallet_contact);
  /** Allows to add new contact(s) to address book
      \param silent - false: show window "Add new contact" and fill contact fields,
                      true:  add directly all contacts to the address book 
                             without showing preview window
  */
  void addToContacts(bool silent, std::list<Contact> &contacts);
  void selectContactItem(QTreeWidgetItem* item);
  void selectIdentityItem(QTreeWidgetItem* item);
  void sideBarSplitterMoved(int pos, int index);

  void openContactGui(int contact_id);
  ContactGui* getContactGui(int contact_id);
  ContactGui* createContactGuiIfNecessary(int contact_id);
  bool isSelectedContactGui(ContactGui* contactGui);  

  void displayDiagnosticLog();
  void setEnabledAttachmentSaveOption(bool enable);
  void setEnabledDeleteOption( bool enable );
  void setEnabledShareContactOption( bool enable );
  void refreshMenuOptions() const;
  void setEnabledMailActions(bool enable);
  void setMailSettings (MailSettings& mailSettings);  
  ContactsTable* getContactsPage();
  void shareContact(QList<const Contact*>& contacts);

  AddressBookModel* getAddressBookModel() const { return _addressbook_model; }

signals:
  void checkSendMailSignal();
protected:
  virtual void closeEvent(QCloseEvent *) override;
  virtual void keyPressEvent(QKeyEvent *) override;

private:
/// IGuiUpdateSink interface description:
  /// \see IGuiUpdateSink interface description.
  virtual void OnReceivedChatMessage(const TContact& sender, const TChatMessage& msg,
    const TTime& timeSent) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnReceivedAuthorizationMessage(const TAuthorizationMessage& msg,
    const TStoredMailMessage& header) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnReceivedMailMessage(const TStoredMailMessage& msg) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSaving() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSaved(const TStoredMailMessage& msg,
    const TStoredMailMessage* overwrittenOne) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageGroupPending(unsigned int count) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessagePending(const TStoredMailMessage& msg,
    const TStoredMailMessage* savedDraftMsg) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageGroupPendingEnd() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSendingStart() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSent(const TStoredMailMessage& pendingMsg,
    const TStoredMailMessage& sentMsg) override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMessageSendingEnd() override;
  /// \see IGuiUpdateSink interface description.
  virtual void OnMissingSenderIdentity(const TRecipientPublicKey& senderId,
    const TPhysicalMailMessage& msg) override;

  /// Only TKeyhoteeApplication can build main window object.
  friend class TKeyhoteeApplication;
  KeyhoteeMainWindow(const TKeyhoteeApplication& mainApp);
  virtual ~KeyhoteeMainWindow();
  /// Helper method to simplify onSidebarSelectionChanged code.
  void activateMailboxPage(Mailbox* mailBox);

  /// \see IModificationsChecker interface description.
  virtual bool canContinue() const override;

private slots:
  // ---------- MenuBar
  // File
  void onExit();
  // Edit
  void onCopy();
  void onCut();
  void onPaste();
  void onSelectAll();
  // Identity
  void onNewIdentity();
  void enableNewMessageIcon();
  void onEnableMining(bool enabled);
  // Mail
  void onSaveAttachement();
  // Contact
  void onSetIcon();
  void onRequestAuthorization();
  void onShareContact();
  // Help
  void onDiagnostic();
  void onAbout();

  void onCanceledNewContact();
  void onSavedNewContact(int idxNewContact);
  void onItemContactRemoved (QTreeWidgetItem&);
  void onRemoveContact ();
  void onClipboardChanged();
  void onFocusChanged(QWidget *old, QWidget *now);
  void onDeleteAuthorizationItem();
  void onItemContextAcceptRequest(QTreeWidgetItem *item);
  void onItemContextDenyRequest(QTreeWidgetItem *item);
  void onItemContextBlockRequest(QTreeWidgetItem *item);
  void onItemAcceptRequest(AuthorizationItem *item);
  void onItemDenyRequest(AuthorizationItem *item);
  void onItemBlockRequest(AuthorizationItem *item);

  void onSidebarSelectionChanged();
  void onSidebarDoubleClicked();

private:
  bool isIdentityPresent();
  void addressBookDataChanged(const QModelIndex& top_left, const QModelIndex& bottom_right,
                              const QVector<int>& roles);
  void searchEditChanged(QString search_string);

  void createContactGui(int contact_id);
  void showContactGui(ContactGui& contact_gui);
  void deleteContactGui(int contact_id);
  void setupStatusBar();
  void notSupported();
  void enableMenu(bool enable);
  /** Allows to stop mail transmission if any before quit.
      Returns true if quit operation can be continued.
  */
  bool stopMailTransmission();
  bool checkSaving() const;
  void showContacts() const;
  void createAuthorizationItem(const TAuthorizationMessage& msg, const TStoredMailMessage& header);
  QTreeWidgetItem* findExistSenderItem(AuthorizationItem::TPublicKey from_key, bool &to_root);
  void showAuthorizationItem(AuthorizationItem *item);
  void deleteAuthorizationItem(AuthorizationItem *item);
  void addContact();
  void processResponse(const TAuthorizationMessage& msg, const TStoredMailMessage& header);
  void loadStoredRequests(bts::bitchat::message_db_ptr request_db);

  /// Class attributes:

  QTreeWidgetItem*                        _identities_root;
  QTreeWidgetItem*                        _mailboxes_root;
  QTreeWidgetItem*                        _wallets_root;
  QTreeWidgetItem*                        _contacts_root;
  QTreeWidgetItem*                        _requests_root;
  QTreeWidgetItem*                        _inbox_root;
  QTreeWidgetItem*                        _drafts_root;
  QTreeWidgetItem*                        _out_box_root;
  QTreeWidgetItem*                        _sent_root;
  QTreeWidgetItem*                        _bitcoin_root;
  QTreeWidgetItem*                        _bitshares_root;
  QTreeWidgetItem*                        _litecoin_root;

  MailboxModel*                           _inbox_model;
  MailboxModel*                           _draft_model;
  MailboxModel*                           _pending_model;
  MailboxModel*                           _sent_model;

  AddressBookModel*                       _addressbook_model;
  bts::addressbook::addressbook_ptr       _addressbook;
  std::unordered_map<int, ContactGui>     _contact_guis;

  QLineEdit*                              _search_edit;
  Ui::KeyhoteeMainWindow*                 ui;
  TConnectionProcessor                    _connectionProcessor;
  Mailbox*                                _currentMailbox;
  MenuEditControl*                        menuEdit;
  /// Set to true when close event processing is in progress (it wasn't yet accepted nor ignored)
  bool                                    _isClosing;
}; //KeyhoteeMainWindow

KeyhoteeMainWindow* getKeyhoteeWindow();
