/*
	simple_calculator_v11.cpp - Simple calculator (elenventh version, proposed exercise 9)

  This program implements a basic expression calculator.
  Input from cin, output from cout.
  The gramman for input is:

  Statement:
    Help
    Constant
    Assign 
    Expression
    Print
    Precision
    SetPrecision
    Quit

  Print:
    ;
    
  Quit:
    quit

  Precision:
    precision

  Precision:
    set precision Number

  Constant:
    const Name = Expression

  Assign:
    Name = Expression

  Expression:
    Term
    Term + Expression
    Term - Expression

  Term:
    Primary
    Primary * Term 
    Primary / Term 
    Primary % Term 

  Primary:
    { List }
    { Columns }
    Function
    Number
    Name
    ( Expression )
    - Primary
    + Primary
    ~ Primary

  Columns:
    { List }
    { List } , Columns

  List:
    Expression 
    Expression , List

  Function:
    FunctionName ( Expression )
    FunctionName ( Expression , Expression )

  FunctionName:
    sin
    cos
    tan
    asin
    acos
    atan
    exp
    pow
    ln
    log10
    log2

  Number:
    floating-point-literal

  Name:
    a string of letters and numbers
  
  Input comes from cin through the Token_stream called ts.
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include <queue>
#include <cmath>
#include <sstream>
#include <map>
#include <iomanip>
#include <limits>

using namespace std;

#include "generic_value.hpp"
using gv=generic_value<double>;

#define DEBUG_FUNC false

#define PROGRAM_NAME "simple_calculator"
constexpr size_t version=13;

inline void error(const string& s)
{
	throw runtime_error(s);
}

inline void error(const string& s, const string& s2) { error(s+s2); }

inline void error(char c, const string& s2) 
{ ostringstream ostr; ostr<<c<<s2; error(ostr.str()); }

struct Token 
{
  enum id
  {
    none,
    quit,
    print,
    number,
    name_token,
    const_token,
    char_token,
    help_token,
    function_token,
    precision_token,
    set, 
    show,
    save,
    load,
    env,
    filename
  };

  id kind;
  char symbol;
  gv value;
  string name;
  double (*function)(double);

  Token() 
  : kind(id::none), symbol(0), value(double(0)), name(), function(nullptr)
  {}

  Token(id tk) 
  : kind(tk), symbol(0), value(double(0)), name(), function(nullptr)
  {}

  Token(id tk, const string& str)
  : kind(tk), symbol(0), value(double(0)), name(str), function(nullptr)
  {}


  Token(char ch) 
  : kind(id::char_token), symbol(ch), value(double(0)), name(), function(nullptr)
  {}

  Token(const gv& val)
  : kind(id::number), symbol(0), value(val), name(), function(nullptr)
  {}

  Token(const string& str) 
  : kind(id::name_token), symbol(0), value(double(0)), name(str), function(nullptr) 
  {}

  Token(const string& str, double (*the_function)(double)) 
  : kind(id::function_token), symbol(0), value(double(0)), name(str), function(the_function) 
  {}

  bool is_symbol(char c) { return ((kind==id::char_token) && (symbol==c)); }
  bool is_number(const gv& v) { return ((kind==id::number) && (value==v)); }
  bool is_name(const string& str) { return ((kind==id::name_token) && (name==str)); }
  bool is_function() { return (kind==id::function_token); }
};

class Token_stream 
{ 
  private:

    queue<Token> buffer; 
    
  public: 
    
    Token_stream() { } 
    Token get(); 
    void unget(Token t) { buffer.push(t); } 
    void ignore();
};

bool is_valid_filename(const string& s)
{
    // 1. No puede estar vacío
    if (s.empty()) return false;

    // 2. No puede empezar ni terminar con '.'
    if (s.front() == '.' || s.back() == '.') return false;

    // 3. Debe contener exactamente un punto (o al menos uno razonable)
    size_t dot_pos = s.find('.');
    if (dot_pos == string::npos) return false;

    // 4. Extensión debe tener al menos 1 caracter
    if (dot_pos == s.length() - 1) return false;

    // 5. Evitar valores numéricos con puntos tipo "3.14"
    bool is_number = true;
    for (char c : s)
        if (!isdigit(c) && c != '.' && c != '-') { is_number = false; break; }
    if (is_number) return false;

    // 6. Caracteres permitidos para nombre de fichero
    for (char c : s)
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-'))
            return false;

    return true;
}

Token Token_stream::get()
{
  if(!buffer.empty()) 
  { 
    auto t=buffer.front(); 
    buffer.pop(); 
    return t; 
  }

  char ch;
  //cin >> ch;
  do { cin.get(ch); } while(isspace(ch));
  switch (ch) 
  {
    case '(': case ')': 
    case '{': case '}':
    case '+': case '-': 
    case '*': case '/': 
    case '%': case '~': 
    case '=': case ',': 
      return Token(ch);

    case ';':
      return Token(Token::id::print);

    case '.': case '0': case '1': case '2': 
    case '3': case '4': case '5': case '6': 
    case '7': case '8': case '9':
    {	
      cin.unget();
      double val;
      cin>>val;
      return Token(gv(val));
    }
    default:
    	if (isalpha(ch)) 
      {
        string s;
        s+=ch;
        while(cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '.' || ch == '_') ) s+=ch;
        cin.unget();

        if(s=="quit") return Token(Token::id::quit);
        if(s=="const") return Token(Token::id::const_token);
        if(s=="help") return Token(Token::id::help_token);
        if(s=="set") return Token(Token::id::set);
        if(s=="precision") return Token(Token::id::precision_token);

        if(s=="show") return Token(Token::id::show);
        if(s=="save") return Token(Token::id::save);
        if(s=="load") return Token(Token::id::load);
        if(s=="env") return Token(Token::id::env);

        if(s=="sin") return Token(s,sin);
        if(s=="cos") return Token(s,cos);
        if(s=="tan") return Token(s,tan);
        if(s=="asin") return Token(s,asin);
        if(s=="acos") return Token(s,acos);
        if(s=="atan") return Token(s,atan);
        if(s=="exp") return Token(s,exp);

        if(s=="pow") return Token(s,nullptr); // WARNING: this is the exception, pow has two arguments
                                            //
        if(s=="ln") return Token(s,log);
        if(s=="log10") return Token(s,log10);
        if(s=="log2") return Token(s,log2);

        //recognize a filename token, where it is a string with a . and the extension
        if (is_valid_filename(s))
          return Token(Token::id::filename, s);


        return Token(s);
    	}
    	error("Bad token");
  }
}



void Token_stream::ignore()
{
  while(!buffer.empty())
  {
    auto t=buffer.front(); buffer.pop();
    if(t.kind==Token::id::quit) return;
  }

  char ch;
  while (cin>>ch)
    if (ch==';') return;
}

struct Value 
{
  string name;
  gv value;
  bool is_const;

  Value() :name{}, value{double(0)}, is_const{false} {}

  Value(const string& n, const gv& v, bool is_constant=false) 
    :name(n), value(v), is_const(is_constant) 
  {}
};

map<string,Value> names;

gv get_value(const string& s)
{
  auto iter=names.find(s);
  if(iter!=names.end()) return iter->second.value;

  error("get: undefined name ",s);
}

void set_value(const string& s, gv v)
{
  auto iter=names.find(s);
  if(iter!=names.end())
  {
    if(iter->second.is_const) error("set: const name ",s);
    iter->second.value=v;
    return;
  }

  error("set: undefined name ",s);
}

bool is_constant(const string& s)
{
  auto iter=names.find(s);
  return (
    (iter!=names.end()) && 
    (iter->second.is_const)
  ); 
}

bool is_declared(const string& s) { return (names.find(s)!=names.end()); }

void define_name(const string& s, const gv& d, bool constant=false)
{ names[s]=Value(s,d,constant); }

Token_stream ts;

constexpr int default_precision=6;
int precision=default_precision;

gv expression();



gv function_name()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(!t.is_function()) error("function name expected");

  Token tt=ts.get();
  if(!tt.is_symbol('(')) error("'(' expected");
  gv v=expression();
  tt=ts.get();
  if(tt.is_symbol(')')) 
  {
    if(t.function) return v.call_function(t.function);
    else error(t.name," needs two arguments");
  }
  else if(!tt.is_symbol(',')) error("')' expected");
  {
    gv vv=expression();
    tt=ts.get();
    if(tt.is_symbol(')')) 
    {
      if(t.name=="pow") return v.call_function(pow,vv); 
      else error(t.name," needs only one argument");
    }
    else error("')' expected");
  }
}

vector<typename gv::matrix_t::value_t::element_t> list()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  vector<typename gv::matrix_t::value_t::element_t> row; 
  Token t;
  do
  {
    auto v=expression();
    row.push_back(v.get<typename gv::scalar_t>());
    t=ts.get();
  } while(t.is_symbol(','));
  ts.unget(t);
  return row;
}

gv columns()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  vector<
    vector<typename gv::matrix_t::value_t::element_t> 
  > rows;

  Token t,tt;
  do
  {
    t=ts.get();
    if(!t.is_symbol('{')) error("'{' expected");
    
    tt=ts.get();
    if(tt.is_symbol('}')) 
      rows.push_back(vector<typename gv::matrix_t::value_t::element_t>()); 
    else 
    {
      ts.unget(tt);

      rows.push_back(list());
      t=ts.get();
      if(!t.is_symbol('}')) error("'}' expected");
    }
    t=ts.get();
  } while(t.is_symbol(','));
  ts.unget(t);

  return gv(typename gv::matrix_t::value_t(rows));
}

gv primary()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();

  if(t.is_function()) { ts.unget(t); return function_name(); }
  else if(t.kind==Token::id::char_token)
  {
    if(t.is_symbol('('))
    {
      gv v=expression();
      t=ts.get();
      if(!t.is_symbol(')')) error("')' expected");
      return v;
    }
    else if(t.is_symbol('{'))
    {
      Token tt=ts.get();
      if(tt.is_symbol('{')) 
      { 
        ts.unget(tt);
        gv v=columns();
        t=ts.get();
        if(!t.is_symbol('}')) error("'}' expected");  
        return v; 
      }
      else if(tt.is_symbol('}')) return gv{typename gv::matrix_t::value_t()};  
      { 
        ts.unget(tt);
        gv v{typename gv::matrix_t::value_t(list())};
        t=ts.get();
        if(!t.is_symbol('}')) error("'}' expected");  
        return v;
      }
    }
    else if(t.is_symbol('-')) return -primary();
    else if(t.is_symbol('+')) return primary();
    else if(t.is_symbol('~')) return ~primary();
  }
  else if(t.kind==Token::id::number) return t.value;
  else if(t.kind==Token::id::name_token) return get_value(t.name);

  error("primary expected");
}

gv term()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  gv left = primary();
  while(true) 
  {
    Token t = ts.get();

    if(t.is_symbol('*')) left=left*primary();
    else if(t.is_symbol('/')) left=left/primary();
    else if(t.is_symbol('%')) left=left%primary();
    else { ts.unget(t); return left; }
  }
}

gv expression()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  gv left = term();
  while(true) 
  {
    Token t = ts.get();

    if(t.is_symbol('+')) left=left+term();
    else if(t.is_symbol('-')) left=left-term();
    else { ts.unget(t); return left; }
  }
}

gv assign()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error ("name expected in assign");
  string name = t.name;
  if (is_constant(name)) error(name," constant cannot be modified"); 
  t=ts.get();

  if(!t.is_symbol('=')) error("= missing in assign of " ,name);

  gv v=expression();

  if(is_declared(name)) 
    set_value(name,v);
  else
    define_name(name,v);

  return v;
}

gv constant_assign()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error("name expected in const assign");
  string name = t.name;
  if(is_declared(name)) error(name," has already been defined"); 
  t=ts.get();

  if(!t.is_symbol('=')) error("= missing in assign of " ,name);

  gv v=expression();

  define_name(name,v,true);

  return v;
}

gv statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();

  switch(t.kind)
  {
    case Token::id::const_token:
      return constant_assign();
      break;

    case Token::id::name_token:
      {
        Token tt=ts.get();
        if(tt.is_symbol('=')) { ts.unget(t); ts.unget(tt); return assign(); }
        else { ts.unget(t); ts.unget(tt); return expression(); }
      }
      break;

    default:
      { ts.unget(t); return expression(); }
  }
}

map<string,gv> functions;

void show_environment()
{
    cout << "Environment variables:\n";
    for(auto& p : names)
    {
        cout << "  " << p.second.name
             << " = " << p.second.value
             << (p.second.is_const ? " (const)" : "")
             << "\n";
    }
}

void save_env(Token tf) {
    // cout tf
    cout << "Saving environment to file: " << tf.name << "\n";
    string file_name = tf.name;
    printf("Saving environment to file: %s\n", file_name.c_str());
    ofstream out(file_name.c_str());
    if (!out) {
        cerr << "Error: cannot open file " << file_name << " for writing.\n";
        return;
    }

    for (map<string, Value>::const_iterator it = names.begin(); it != names.end(); ++it) {
        const string& name = it->first;
        const gv& g = it->second.value;
        bool is_const = it->second.is_const;

        // Convertir valor a string
        std::ostringstream oss;
        g.to_stream(oss);
        std::string text = oss.str();

        // Detectar tipo por impresión
        if (!text.empty() && text[0] == '{') {
            // Es matriz → guardamos texto íntegro
            out << name << " M " << is_const << " " << text << "\n";
        } else {
            // Es escalar
            out << name << " S " << is_const << " " << text << "\n";
        }
    }

    cout << "Environment saved to " << file_name << "\n";
}

void load_env(Token tf) {
    string file_name = tf.name;
    ifstream in(file_name.c_str());
    if (!in) {
        cerr << "Error: cannot open file " << file_name << " for reading.\n";
        return;
    }

    names.clear();

    string name;
    char type;
    bool is_const;

    while (in >> name >> type >> is_const) {

        if (type == 'S') {
            double x;
            in >> x;
            names[name] = Value(name, gv(x), is_const);
        }

        else if (type == 'M') {
          std::string matrix_text;
          std::getline(in, matrix_text);

          while (!matrix_text.empty() && isspace(matrix_text[0]))
              matrix_text.erase(0, 1);

          // Crear un stream con la matriz y añadir un ';'
          matrix_text += ";";

          std::istringstream iss(matrix_text);

          // --- Crear token stream temporal ---
          Token_stream ts_temp;
          
          // --- Redirigir SOLO el buffer del ts_temp, NO cin ---
          // Modify Token_stream to accept custom input stream if needed
          // but easier is wrap parsing in an isolated environment:

          std::streambuf* old_cin = std::cin.rdbuf(iss.rdbuf());

          // Backup global ts
          Token_stream ts_backup = ts;
          ts = Token_stream();

          gv m = statement();

          // Restaurar estado
          ts = ts_backup;
          std::cin.rdbuf(old_cin);

          names[name] = Value(name, m, is_const);
  }

    }

    cout << "Environment loaded from " << file_name << "\n";
}


void clean_up_mess()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
	ts.ignore();
}

void help()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  cout
    <<" "<<PROGRAM_NAME<<" - v"<<version<<" - MUSIANI - Programación y Prototipado"
    <<"\n"
    <<"\n This  is  a simple  calculator  which  accepts  aritmetic"
    <<"\n expressions  with  numeric  literals and simbolic  values"
    <<"\n like constants and variables."
    <<"\n"
    <<"\n Allows also the use of the following fundamental functions:"
    <<"\n"
    <<"\n   sin, cos, tan, asin, acos, atan"
    <<"\n   exp, pow, ln, log10, log2"
    <<"\n"
    <<"\nIt is a matricial calculator, and accepts matrix literals with the following"
    <<"\nnotation (spaces, line feeds and carry returns are skipped):"
    <<"\n"
    <<"\n   {{1,2},{-2,-1}};"
    <<"\n   or"
    <<"\n   {"
    <<"\n     { 122.000000, 244.000000, 366.000000 },"
    <<"\n     { 244.000000, 488.000000, 732.000000 },"
    <<"\n     { 366.000000, 732.000000, 1098.000000 }"
    <<"\n   }"
    <<"\n   or"
    <<"\n   { 0.540302, 0.004426, 0.004426, -0.999961, -0.013277 }"
    <<"\n"
    <<"\n Notation is very simple, as shown in the following examples:"
    <<"\n "
    <<"\n   2+3*7-(8-3.2)-1/3; (aritmetic expression)"
    <<"\n   a=3; (assign of a value to a variable)"
    <<"\n   const pi=3.141592; (assign of a value to a constant)"
    <<"\n   2*pi-a; (arithmetic expression with variables and constants)"
    <<"\n   sin(2*pi/4); (arithmetic expression with functions)"
    <<"\n   const e=exp(1); (arithmetic expression with functions)" 
    <<"\n   ln(2*e/pi); (arithmetic expression with functions)"
    <<"\n   pow(3,2); (arithmetic expression with functions)"
    <<"\n   v={1,1,2,3,5,8,13,21,34} (assign a line vector to a varible)"
    <<"\n   exp(v) (arithmetic expression with functions and a line vector)"
    <<"\n   a={"
    <<"\n     { 122.000000, 244.000000, 366.000000 },"
    <<"\n     { 244.000000, 488.000000, 732.000000 },"
    <<"\n     { 366.000000, 732.000000, 1098.000000 }"
    <<"\n   } (a literal matrix assigned to a varible)"<<"\n"
    <<"\n"
    <<"\n Mind that all expressions should be finished with a symbol ';'."
    <<"\n For finishing the execution type \"quit\"."
    <<"\n"
    <<"\n Additional commands: "
    <<"\n"
    <<"\n   precision; (shows how many fractional digits are used for showing calculator's results)"
    <<"\n   set precision <numeric_expression>; (changes calculator's precision)"
    <<"\n"
  ; 
}

void precision_statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  cout
    <<" precision digits: "<<precision<<"\n"
  ;
}

void set_precision()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::precision_token) error("precision keyword expected");

  gv d=expression();
  precision=d.get<gv::scalar_t>();

  cout
    <<" precision set to "<<precision<<" digits\n"
  ;
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  while(true) 
  try 
  {
    cout<<prompt;
    Token t=ts.get();
    while (t.kind==Token::id::print) t=ts.get();
    if(t.kind==Token::id::quit) return;
    if(t.kind==Token::help_token) { help(); continue; }
    if(t.kind==Token::precision_token) { precision_statement(); continue; }
    if(t.kind==Token::set) { set_precision(); continue; }
    if (t.kind==Token::show) //tomar el siguiente token y ver si es env
    {
        Token tt=ts.get();
        if (tt.kind==Token::env) show_environment();
        else { ts.unget(tt); error("env should follow show token"); }
        continue;
    }
    if (t.kind==Token::save) //tomar el siguiente token y ver si es filename
    {
        Token tt=ts.get();
        if (tt.kind==Token::env) 
        {
            Token tf=ts.get();
            if (tf.kind==Token::filename) save_env(tf);
            else { ts.unget(tf); error("filename should follow env token"); }
        }
        else { ts.unget(tt); error("env should follow save token"); }
        continue;
    }
    if (t.kind==Token::load) //tomar el siguiente token y ver si es filename
    {
        Token tt=ts.get();
        if (tt.kind==Token::env) 
        {
            Token tf=ts.get();
            if (tf.kind==Token::filename) load_env(tf.name);
            else { ts.unget(tf); error("filename should follow env token"); }
        }
        else { ts.unget(tt); error("env should follow load token"); }
        continue;
    }

    ts.unget(t);
    auto the_result=statement();
    cout<<fixed<<setprecision(precision)<<result<<the_result<<endl;
  }
  catch(runtime_error& e) 
  {
    cerr<<e.what()<< endl;
    clean_up_mess();
  }
  catch(logic_error& e) 
  {
    cerr<<e.what()<< endl;
    clean_up_mess();
  }
}

int main()
try 
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
  help();

  calculate();
  return 0;
}
catch (exception& e) {
  cerr<<"exception: "<<e.what()<<endl;
  char c;
  while((cin>>c) && (c!=';')) ;
  return 1;
}
catch (...) {
  cerr << "exception\n";
  char c;
  while((cin>>c) && (c!=';')) ;
  return 2;
}
