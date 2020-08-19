#pragma once

#include <iostream>
#include <string>

enum Verbosity { Fatal, Error, Warn, Info, Debug, Trace };
extern Verbosity verbosity;

void fatal(char *);
void error(char *);
void warn(char *);
void info(char *);
void debug(char *);
void trace(char *);

void fatal(std::string);
void error(std::string);
void warn(std::string);
void info(std::string);
void debug(std::string);
void trace(std::string);

std::ostream &lfatal();
std::ostream &lerror();
std::ostream &lwarn();
std::ostream &linfo();
std::ostream &ldebug();
std::ostream &ltrace();
