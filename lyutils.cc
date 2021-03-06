#include <assert.h>
#include <ctype.h>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include "auxlib.h"
#include "lyutils.h"
#include "stringset.h"
#include "yyparse.h"
using namespace std;

astree* yyparse_astree = NULL;
int scan_linenr = 1;
int scan_offset = 0;
bool scan_echo = false;
vector<string> included_filenames;

const string* scanner_filename (int filenr) {
   return &included_filenames.at(filenr);
}

void scanner_newfilename (const char* filename) {
   included_filenames.push_back (filename);
}

void scanner_newline (void) {
   ++scan_linenr;
   scan_offset = 0;
}

void scanner_setecho (bool echoflag) {
   scan_echo = echoflag;
}


void scanner_useraction (void) {
   if (scan_echo) {
      if (scan_offset == 0) printf (";%5d: ", scan_linenr);
      printf ("%s", yytext);
   }
   scan_offset += yyleng;
}

void yyerror (const char* message) {
   assert (not included_filenames.empty());
   errprintf ("%:%s: %d: %s\n",
              included_filenames.back().c_str(),
              scan_linenr, message);
}

void scanner_badchar (unsigned char bad) {
   char char_rep[16];
   sprintf (char_rep, isgraph (bad) ? "%c" : "\\%03o", bad);
   errprintf ("%:%s: %d: invalid source character (%s)\n",
              included_filenames.back().c_str(),
              scan_linenr, char_rep);
   //do something here
}

void scanner_badtoken (char* lexeme) {
   errprintf ("%:%s: %d: invalid token (%s)\n",
              included_filenames.back().c_str(),
              scan_linenr, lexeme);
}

int yylval_token (int symbol){
    int offset = scan_offset - yyleng;
    yylval = new_astree (symbol, included_filenames.size() - 1,
                         scan_linenr, offset, yytext);
    intern_stringset(yytext);
    outfile_tok <<"  "<<left<<
            setw(2)<<setfill(' ')<<yylval->filenr <<
            setw(3)<<right<<setfill(' ')<<yylval->linenr <<"." <<
            setw(3)<<setfill('0')<<yylval->offset << "  "<<
            setw(3)<<right<<setfill(' ')<<yylval->symbol <<" " <<
            setw(13)<<left<<setfill(' ')<<
            get_yytname(yylval->symbol) << " " <<'(' << *yylval->lexinfo << ')' << endl;
    return symbol;
}

astree* new_parseroot (void) {
   yyparse_astree = new_astree (TOK_ROOT, 0, 0, 0, "<<ROOT>>");
   return yyparse_astree;
}

void scanner_include (void){
    scanner_newline();
    char filename[strlen (yytext) + 1];
    int linenr;
    int scan_rc = sscanf (yytext, "# %d \"%[^\"]\"",
                          &linenr, filename);
    if (scan_rc != 2){
        errprintf ("%: %d: [%s]: invalid directive, ignored\n",
                   scan_rc, yytext);
    }
    else{
        scanner_newfilename (filename);
        scan_linenr = linenr - 1;
        outfile_tok << "# " << linenr <<" \""<<filename<<"\""<<endl;
        DEBUGF ('m', "filename=%s, scan_linenr=%d\n",
                included_filenames.back().c_str(), scan_linenr);
    }
}

RCSC("$Id: lyutils.cc,v 1.3 2013-10-11 18:56:52-07 - - $")

