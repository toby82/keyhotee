#ifndef __UTILS_HPP
#define __UTILS_HPP

/// Set of helper functions used to backend data text conversions.

#include <fc/time.hpp>
#include <fc/crypto/elliptic.hpp>

#include <QDateTime>
#include <QString>

#include <vector>

namespace bts
{
namespace addressbook
{
struct contact;
struct wallet_contact;
} ///namespace addressbook
} /// namespace bts

namespace Utils
{

inline
QDateTime toQDateTime(const fc::time_point_sec& time_in_seconds)
  {
  QDateTime date_time;
  date_time.setTime_t(time_in_seconds.sec_since_epoch());
  return date_time;
  }

enum TContactTextFormatting
  {
  /// Will result in producing: kID
  KEYHOTEE_IDENTIFIER      = 0x1,
  /// Will result in producing: AliasFn AliasLn
  CONTACT_ALIAS_FULL_NAME  = 0x2,
  /// Will result in producing: AliasFn AliasLn <kID>
  FULL_CONTACT_DETAILS = CONTACT_ALIAS_FULL_NAME | KEYHOTEE_IDENTIFIER
  };

/** Allows to convert given public key to textual form.
    If given key matches to one of known contacts (incl. created own identities) returned string
    can be built in a form defined by contactFormatting parameter.
    if given key is unknown, function returns just its textual form (as base58).

    \param pk - input public key. Must be valid
    \param contactFormatting - settings related to contact textual fomatting.
                \see TContactTextFormatting definition for details.
    \param matchingContact - optional, output parameter. Can be null. Will point to found contact
                info or if contact is unknown will be just filled with input public key.
*/
QString toString(const fc::ecc::public_key& pk, TContactTextFormatting contactFormatting,
  bts::addressbook::contact* matchingContact = nullptr);

bool matchContact(const fc::ecc::public_key& pk, bts::addressbook::wallet_contact* matchedContact);

/** Allows to convert list of keys into textual form, separated by given character.
    \see above toString description to single key conversion details.
*/
QString makeContactListString(const std::vector<fc::ecc::public_key>& key_list, char separator = ',',
  TContactTextFormatting contactFormatting = TContactTextFormatting::KEYHOTEE_IDENTIFIER);
/* ltrim of QString
*/
QString lTrim(QString const &str);

} ///namespace Utils

#endif /// __UTILS_HPP

