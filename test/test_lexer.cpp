/*
  Copyright 2015-2018 Giuseppe Lipari
  email: giuseppe.lipari@univ-lille.fr
  
  This file is part of TiPa.

  TiPa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  TiPa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.
  
  You should have received a copy of the GNU General Public License
  along with TiPa. If not, see <http://www.gnu.org/licenses/>
 */
#include "catch.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>
#include <sstream>

#include <lexer.hpp>

using namespace std;
using namespace tipa;

#define LEX_EQ       3
#define LEX_QUOTES   4
#define LEX_TASK     5
#define LEX_SYS      6
#define LEX_PIPELINE 7


static void check(ahead_lexer &lex, vector<token_id> &results) 
{
    for (int i=0; i<results.size(); i++) {
        auto r = lex.get_token();
        INFO(i);
        REQUIRE( r.first == results[i] );
    } 
    auto r = lex.get_token();
    REQUIRE(r.first == LEX_ERROR);
}

TEST_CASE("struct from file", "[lexer]")
{
    ifstream file("struct.txt");
    if (!file.is_open()) FAIL("File struct.txt not found");
    ahead_lexer lex({tk_int, 
            {LEX_TASK, "task"}, 
            {LEX_SYS, "sys"}, 
            {LEX_PIPELINE, "pipeline"}, 
                tk_ident, 
                    tk_op_par, 
                    tk_cl_par, 
                    tk_semicolon, 
                    tk_colon, 
                    tk_op_br, 
                    tk_cl_br,
                    {LEX_EQ, "="},
                    {LEX_QUOTES, "\""}});


    std::vector<token_id> results = {
        LEX_SYS, tk_op_par.get_name(), tk_ident.get_name(), tk_cl_par.get_name(), 
        tk_op_br.get_name(), LEX_TASK, tk_op_par.get_name(), tk_ident.get_name(), tk_cl_par.get_name(), 
        tk_op_br.get_name(), tk_ident.get_name(), LEX_EQ, tk_int.get_name(), tk_semicolon.get_name(),
        tk_ident.get_name(), LEX_EQ, tk_int.get_name(), tk_semicolon.get_name(),
        tk_cl_br.get_name(), tk_semicolon.get_name(),
        tk_cl_br.get_name(), tk_semicolon.get_name()
    };

    lex.set_stream(file);
    check(lex, results);
}

TEST_CASE("save and restore", "[lexer]")
{
    ahead_lexer lex({tk_int, tk_ident});

    SECTION("simple") {
        stringstream str("123 abc 789");
        lex.set_stream(str);
        token_val tk = lex.get_token();
        REQUIRE(tk.second == "123");
        lex.save();
        tk = lex.get_token();
        REQUIRE(tk.second == "abc");
        lex.restore();
        tk = lex.get_token();
        REQUIRE(tk.second == "abc");
        tk = lex.get_token();
        REQUIRE(tk.second == "789");    
    }
    SECTION("multi-line") {
        stringstream str("123\nabc\n789\ndef");
        lex.set_stream(str);
        token_val tk = lex.get_token();
        REQUIRE(tk.second == "123");
        INFO("Pos: " << lex.get_pos().first << ", " << lex.get_pos().second);
        lex.save();
        tk = lex.get_token();
        REQUIRE(tk.second == "abc");
        tk = lex.get_token();
        REQUIRE(tk.second == "789");    
        lex.restore();
        INFO("After restore, pos: " << lex.get_pos().first << ", " << lex.get_pos().second);
        tk = lex.get_token();
        REQUIRE(tk.second == "abc");
        tk = lex.get_token();
        REQUIRE(tk.second == "789");    
        tk = lex.get_token();
        REQUIRE(tk.second == "def");    
    }
}

TEST_CASE("Skip spaces", "[lexer]") 
{
    ahead_lexer lex({tk_int, tk_ident});
    stringstream str("123 abc \n 789");
    lex.set_stream(str);
    token_val tk = lex.get_token();
    REQUIRE(tk.second == "123");
    tk = lex.get_token();
    REQUIRE(tk.second == "abc");
    tk = lex.get_token();
    REQUIRE(tk.second == "789");
}


