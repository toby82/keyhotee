#include "authorization.hpp"
#include "ui_authorization.h"

#include <fc/reflect/variant.hpp>
#include "public_key_address.hpp"
#include "AddressBookModel.hpp"
#include "Contact.hpp"

#include <QMessageBox>
#include <QToolBar>

Authorization::Authorization(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Authorization)
{
  ui->setupUi(this);

  _address_book = nullptr;
  _owner_item = nullptr;

  ui->keyhoteeidpubkey->setMode(KeyhoteeIDPubKeyWidget::ModeWidget::RequestAuthorization);
  ui->keyhoteeidpubkey->setEditable(false);
  ui->keyhoteeidpubkey->showCopyToClipboard(false);

  _toolbar = new QToolBar(ui->toolbar_container);
  QGridLayout* grid_layout = new QGridLayout(ui->toolbar_container);
  grid_layout->setContentsMargins(0, 0, 0, 0);
  grid_layout->setSpacing(0);
  ui->toolbar_container->setLayout(grid_layout);
  grid_layout->addWidget(_toolbar, 0, 0);
  
  _accept = new QAction( QIcon( ":/images/request_accept.png"), tr("Accept"), this);
  _deny = new QAction( QIcon( ":/images/request_deny.png"), tr("Deny"), this);  
  _block = new QAction( QIcon(":/images/request_block.png"), tr("Block"), this);
  
  _toolbar->addAction(_accept);  
  _toolbar->addAction(_deny);
  _toolbar->addAction(_block);

  QLabel *label = new QLabel((tr("     Response for authorization request")));
  _toolbar->addWidget (label);
  QFont font;  
  font.setBold(true);
  font.setPointSize (16);
  label->setFont (font);

  connect(_accept, &QAction::triggered, this, &Authorization::onAccept);
  connect(_deny, &QAction::triggered, this, &Authorization::onDeny);
  connect(_block, &QAction::triggered, this, &Authorization::onBlock);
  connect(ui->add_contact , &QGroupBox::toggled, this, &Authorization::onAddAsNewContact);
  connect(ui->keyhoteeidpubkey, &KeyhoteeIDPubKeyWidget::currentState, this, &Authorization::onStateWidget);

  // setting the background color of the frame so that the window looked like a window "create new contact"
  QPalette palette = ui->frame->palette();
  palette.setColor(backgroundRole(), QGuiApplication::palette().base().color());
  ui->frame->setPalette(palette);
}

Authorization::~Authorization()
{
  delete ui;
}

void Authorization::setAddressBook(AddressBookModel* addressbook)
{
  _address_book = addressbook;
}

void Authorization::setMsg(const TDecryptedMessage& msg)
{
  _msg = msg;
  TRequestMessage  reqmsg = msg.as<TRequestMessage>();

  TPublicKey public_key = *_msg.from_key;
  std::string public_key_string = public_key_address(public_key.serialize());
  ui->keyhoteeidpubkey->setPublicKey(public_key_string.c_str());
  ui->keyhoteeidpubkey->setKeyhoteeID(reqmsg.from_keyhotee_id.c_str());

  ui->first_name->setText( reqmsg.from_first_name.c_str() );
  ui->last_name->setText(reqmsg.from_last_name.c_str() );
  
  ui->check_box_chat->setChecked(reqmsg.request_param & 0x01);
  ui->check_box_mail->setChecked(reqmsg.request_param>>1 & 0x01);
  ui->extend_public_key->setChecked(reqmsg.request_param>>8 & 0x01);

  ui->message->setText(reqmsg.greeting_message.c_str());

  _extend_pub_key = reqmsg.extended_pub_key;
}

void Authorization::setOwnerItem(AuthorizationItem* item)
{
  _owner_item = item;
}


void Authorization::onAccept()
{
  addAsNewContact();
  acceptExtendedPubKey();
  close();
  emit itemAcceptRequest(_owner_item);
}

void Authorization::onDeny()
{
  close();
  emit itemDenyRequest(_owner_item);
}

void Authorization::onBlock()
{
  close();
  emit itemBlockRequest(_owner_item);
}

void Authorization::addAsNewContact()
{
  if(ui->add_contact->isEnabled() && ui->add_contact->isChecked())
  {
    Contact new_conntact;
    new_conntact.first_name       = ui->first_name->text().toUtf8().constData();
    new_conntact.last_name        = ui->last_name->text().toUtf8().constData();
    new_conntact.dac_id_string    = ui->keyhoteeidpubkey->getKeyhoteeID().toUtf8().constData();     // can better directly from the message?
    new_conntact.public_key       = *_msg.from_key;
    new_conntact.privacy_setting  = bts::addressbook::secret_contact;
    new_conntact.setIcon(QIcon(":/images/user.png"));

    _address_book->storeContact(new_conntact);
  }
}

void Authorization::acceptExtendedPubKey()
{
  if(ui->extend_public_key->isChecked())
  {
    auto app = bts::application::instance();
    auto profile = app->get_profile();
    auto addressbook = profile->get_addressbook();
    auto contact = addressbook->get_contact_by_public_key( ui->keyhoteeidpubkey->getPublicKey() );
    contact->send_trx_address = _extend_pub_key;
    addressbook->store_contact(*contact);

    // send the key to the sender *******************************
  }
}

void Authorization::onAddAsNewContact(bool checked)
{
  if(ui->extend_public_key->isChecked() &&
      ui->add_contact->isEnabled() && !ui->add_contact->isChecked())
  {
    QMessageBox::information(this, tr("Information"), tr("To accept the Extended Public Key is necessary to add a new contact."));
    ui->add_contact->setChecked(true);
  }
}

void Authorization::onStateWidget(KeyhoteeIDPubKeyWidget::CurrentState state)
{
  switch(state)
  {
    case KeyhoteeIDPubKeyWidget::CurrentState::InvalidData:
    case KeyhoteeIDPubKeyWidget::CurrentState::IsStored:
      ui->add_contact->setEnabled(false);
      break;
    case KeyhoteeIDPubKeyWidget::CurrentState::OkKeyhoteeID:
    case KeyhoteeIDPubKeyWidget::CurrentState::OkPubKey:
      ui->add_contact->setEnabled(true);
      break;
    default:
      assert(false);
  }
}

