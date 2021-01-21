/*
AUTORZY:
Grzegorz Zaleski - gz418494
Konrad Korczyński - kk418331
*/

#include "encstrset.h"
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace
{
    //---------------------|||ZMIENNE GLOBALNE I DEFINICJE|||--------------------//

// Aktywacja/dezaktywacja trybu debugującego.
#ifdef NDEBUG
    static const bool debug = false;
#else
    static const bool debug = true;
#endif

    using std::cerr;
    using std::endl;
    using std::string;
    using std::to_string;
    using std::unique_ptr;
    using std::unordered_map;
    using std::unordered_set;

    using data_map = unordered_map<unsigned long, unordered_set<string>>;

    // Numer identyfikatora kolejnego zbioru.
    unsigned long newSetIndex = 0;

    // Ochrona przed fiaskiem statycznej inicializacji.
    data_map &database()
    {
        static unique_ptr<data_map> database(new data_map());
        return *database;
    }

    // Tablica znaków systemu szesnastkowego.
    constexpr int HEX_LEN = 16;
    char const hex_chars[HEX_LEN] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    //--------------------|||F. POMOCNICZE|||--------------------------//

    //---------------------CYPHRIFY-----------------------------------//
    // Zwraca zaszyfrowany za pomocą operacji XOR, wykonanej na value oraz key, ciąg znaków.
    string cyphrify(string &value, string &key)
    {
        if (key.empty())
            return value;

        string res = "";
        for (unsigned int i = 0; i < value.length(); ++i)
            res += value[i] ^ key[i % key.length()];

        return res;
    }

    //---------------------CERR HEX---------------------------------//
    // Zwraca ciąg znaków odpowiadający postaci szesnastkowej ciągu cypher.
    string cerr_hex(string &cypher)
    {
        string hexString = "";
        hexString += "\"";
        for (unsigned int i = 0; i < cypher.length(); ++i)
        {
            hexString += hex_chars[(cypher[i] & 0xF0) >> 4];
            hexString += hex_chars[cypher[i] & 0x0F];

            if (i + 1 != cypher.length())
                hexString += " ";
        }
        hexString += "\"";

        return hexString;
    }

    //---------------------FUNCTION LOG------------------------------//
    // Zbiór przeładowanych funkcji wypisujących wiadomości debugujące:
    // Dla funkcji z argumentem id,
    void function_log(const string &func, unsigned long id, const string &comment)
    {
        if (debug)
            cerr << func << "(" << id << ")\n"
                 << func << ": set #" << id << comment << endl;
    }

    // Dla funkcji z argumentami id, val, key,
    void function_log(const string &func, unsigned long id, const string &val, const char *key, const string &comment)
    {
        if (debug)
        {
            if (key == NULL)
                cerr << func << "(" << id << ", \"" << val << "\", NULL)\n";
            else
                cerr << func << "(" << id << ", \"" << val << "\", \"" << key << "\")\n";
            cerr << func << ": set #" << id << comment << endl;
        }
    }

    // Dla funkcji z argumentami src_id, dst_id.
    void function_log(const string &func, unsigned long src_id, unsigned long dst_id, const string &comment)
    {
        if (debug)
            cerr << func << "(" << src_id << ", " << dst_id << ")\n"
                 << func << ":" << comment << endl;
    }

    //-------------------NULL VAL-----------------------------------//
    // Funkcja zajmująca się sytuacją (value = NULL). [WRAPPER]
    bool null_val(const string &func, unsigned long id, const char *key)
    {
        if (debug)
        {
            if (key == NULL)
                cerr << func << "(" << id << ", NULL, NULL)\n";
            else
                cerr << func << "(" << id << ", NULL, \"" << key << "\")\n";

            cerr << func << ": invalid value (NULL)\n";
        }

        return false;
    }

    //-------------------ARGS PREPROCESS------------------------//
    // Funckja zajmująca się szyfrowaniem i konwersją z (const char * -> string) w funkcjach głównych. [WRAPPER]
    void args_preprocess(const char *value, const char *key, string &value_str, string &key_str, string &hashed_value)
    {
        value_str = string(value);
        key_str = "";

        if (key != NULL)
            key_str = string(key);

        hashed_value = cyphrify(value_str, key_str);
    }

} //namespace

/////////////////////////ENCSTRSET NEW/////////////////////////////
unsigned long jnp1::encstrset_new()
{
    // Stworzenie nowego zbioru w bazie.
    database()[newSetIndex];

    // Odpowiednik function_log(); przeładowanie dla jednego przypadku było nieopłacalne.
    if (debug)
        cerr << __FUNCTION__ << "()\n"
             << __FUNCTION__ << ": set #" << newSetIndex << " created\n";

    // Zwiększenie identyfikatora następnego zbioru.
    return newSetIndex++;
}

/////////////////////////ENCSTRSET DELETE////////////////////////////////////////
void jnp1::encstrset_delete(unsigned long id)
{
    if (database().erase(id) == 1)
        function_log(__FUNCTION__, id, " deleted");
    else
        function_log(__FUNCTION__, id, " does not exist");
}

/////////////////////////ENCSTRSET SIZE//////////////////////////////////////////
size_t jnp1::encstrset_size(unsigned long id)
{
    if (database().find(id) == database().end())
    {
        function_log(__FUNCTION__, id, " does not exist");
        return 0;
    }
    function_log(__FUNCTION__, id, " contains " + to_string(database().find(id)->second.size()) + " element(s)");

    return database().find(id)->second.size();
}

/////////////////////////ENCSTRSET INSERT////////////////////////////////////////
bool jnp1::encstrset_insert(unsigned long id, const char *value, const char *key)
{
    // Dla value == NULL od razu kończymy funkcje z wiadomością o błędzie.
    if (value == NULL)
        return null_val(__FUNCTION__, id, key);

    // Szyfrujemy oraz konwertujemy dane na typ string.
    string value_str, key_str, hashed_value;
    args_preprocess(value, key, value_str, key_str, hashed_value);

    // Nie znaleziono zbioru o identyfikatorze id.
    if (database().find(id) == database().end())
    {
        function_log(__FUNCTION__, id, value_str, key, " does not exist");
        return false;
    }
    // W zbiorze o identyfikatorze id, znajduje się już element o wartości hashed_value.
    else if (database()[id].find(hashed_value) != database()[id].end())
    {
        function_log(__FUNCTION__, id, value_str, key, ", cypher " + cerr_hex(hashed_value) + " was already present");
        return false;
    }
    // Dodanie jest prawidłowe.
    else
    {
        database()[id].insert(hashed_value);

        function_log(__FUNCTION__, id, value_str, key, ", cypher " + cerr_hex(hashed_value) + " inserted");
        return true;
    }
}

/////////////////////////ENCSTRSET REMOVE////////////////////////////////////////
bool jnp1::encstrset_remove(unsigned long id, const char *value, const char *key)
{
    // Dla value == NULL od razu kończymy funkcje z wiadomością o błędzie.
    if (value == NULL)
        return null_val(__FUNCTION__, id, key);

    // Szyfrujemy oraz konwertujemy dane na typ string.
    string value_str, key_str, hashed_value;
    args_preprocess(value, key, value_str, key_str, hashed_value);

    // Zbiór o identyfikatorze id nie istnieje.
    if (database().find(id) == database().end())
    {
        function_log(__FUNCTION__, id, value_str, key, " does not exist");
        return false;
    }
    // W zbiorze o identyfikatorze id, nie znajduję się element równy hashed_value.
    else if (database().find(id)->second.find(hashed_value) == database().find(id)->second.end())
    {
        function_log(__FUNCTION__, id, value_str, key, ", cypher " + cerr_hex(hashed_value) + " was not present");
        return false;
    }
    // Usunięcie może przebiec bez problemu.
    database().find(id)->second.erase(hashed_value);
    function_log(__FUNCTION__, id, value_str, key, ", cypher " + cerr_hex(hashed_value) + " removed");

    return true;
}

/////////////////////////ENCSTRSET TEST/////////////////////////////////////////
bool jnp1::encstrset_test(unsigned long id, const char *value, const char *key)
{
    // Dla value == NULL od razu kończymy funkcje z wiadomością o błędzie.
    if (value == NULL)
        return null_val(__FUNCTION__, id, key);

    // Szyfrujemy oraz konwertujemy dane na typ string.
    string value_str, key_str, hashed_value;
    args_preprocess(value, key, value_str, key_str, hashed_value);

    // Znaleziono element równy hashed_value w zbiorze o identyfikatorze id.
    if (database().find(id) != database().end() && database().find(id)->second.find(hashed_value) != database().find(id)->second.end())
    {
        function_log(__FUNCTION__, id, value_str, key, ", cypher " + cerr_hex(hashed_value) + " is present");
        return true;
    }
    // Nie znaleiono elementu równego hashed_value w zbiorze o identyfikatorze id.
    else if (database().find(id) != database().end())
    {
        function_log(__FUNCTION__, id, value_str, key, ", cypher " + cerr_hex(hashed_value) + " is not present");
        return false;
    }
    // Nie znaleziono zbioru o identyfikatorze id.
    function_log(__FUNCTION__, id, value_str, key, " does not exist");
    return false;
}

/////////////////////////ENCSTRSET CLEAR/////////////////////////////////////////
void jnp1::encstrset_clear(unsigned long id)
{
    // Zbiór, który chcemy usunąć istnieje.
    if (database().erase(id) == 1)
    {
        function_log(__FUNCTION__, id, " cleared");
        // Zamieniamy aktualny zbiór na nowy pusty zbiór.
        database()[id] = unordered_set<string>();
    }
    else
        function_log(__FUNCTION__, id, " does not exist");
}

/////////////////////////ENCSTRSET COPY/////////////////////////////////////////
void jnp1::encstrset_copy(unsigned long src_id, unsigned long dst_id)
{
    // Zbiór źródłowy nie istnieje.
    if (database().find(src_id) == database().end())
        function_log(__FUNCTION__, src_id, dst_id, " set #" + to_string(src_id) + " does not exist");
    // Zbiór docelowy nie istnieje.
    else if (database().find(dst_id) == database().end())
        function_log(__FUNCTION__, src_id, dst_id, " set #" + to_string(dst_id) + " does not exist");
    else
    {
        // Pierwsza informacja debugująca.
        if (debug)
            cerr << __FUNCTION__ << "(" << src_id << ", " << dst_id << ")\n";

        // Sekcja kopiująca.
        for (auto ele : database()[src_id])
        {
            // Element ze zbioru źródłowego nie znajduje się w zbiorze docelowym.
            if (database()[dst_id].find(ele) == database()[dst_id].end())
            {
                database()[dst_id].insert(ele);
                if (debug)
                    cerr << __FUNCTION__ << ": cypher " + cerr_hex(ele) + " copied from set #" + to_string(src_id) + " to set #" + to_string(dst_id) << endl;
            }
            else if (debug)
                cerr << __FUNCTION__ << ": copied cypher " + cerr_hex(ele) + " was already present in set #" + to_string(dst_id) << endl;
        }
    }
}
