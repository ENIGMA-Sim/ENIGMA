#include "utils/XMLWriter.hpp"
#include <iostream>
#include <sstream>

namespace enigma {

XMLWriter::XMLWriter(const std::string& filename) 
    : file_(filename), indentLevel_(0) {
    if (!file_.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
}

XMLWriter::~XMLWriter() {
    close();
}

void XMLWriter::writeDeclaration() {
    file_ << "<?xml version='1.0'?>\n";
}

void XMLWriter::writeComment(const std::string& comment) {
    writeIndent();
    file_ << "<!-- " << comment << " -->\n";
}

void XMLWriter::startElement(const std::string& name) {
    writeIndent();
    file_ << "<" << name << ">\n";
    elementStack_.push_back(name);
    indentLevel_++;
}

void XMLWriter::startElement(const std::string& name, 
                              const std::map<std::string, std::string>& attributes) {
    writeIndent();
    file_ << "<" << name;
    
    for (const auto& [key, value] : attributes) {
        file_ << " " << key << "=\"" << escapeXML(value) << "\"";
    }
    
    file_ << ">\n";
    elementStack_.push_back(name);
    indentLevel_++;
}

void XMLWriter::endElement(const std::string& name) {
    if (elementStack_.empty() || elementStack_.back() != name) {
        throw std::runtime_error("XML element closing error: " + name);
    }
    
    indentLevel_--;
    elementStack_.pop_back();
    writeIndent();
    file_ << "</" << name << ">\n";
}

void XMLWriter::writeElement(const std::string& name, const std::string& content) {
    writeIndent();
    file_ << "<" << name << ">" << escapeXML(content) << "</" << name << ">\n";
}

void XMLWriter::writeEmptyElement(const std::string& name, 
                                   const std::map<std::string, std::string>& attributes) {
    writeIndent();
    file_ << "<" << name;
    
    for (const auto& [key, value] : attributes) {
        file_ << " " << key << "=\"" << escapeXML(value) << "\"";
    }
    
    file_ << "/>\n";
}

void XMLWriter::writeRaw(const std::string& content) {
    file_ << content;
}

void XMLWriter::close() {
    if (file_.is_open()) {
        if (!elementStack_.empty()) {
            std::cerr << "Warning: Unclosed XML elements at finish\n";
        }
        file_.close();
    }
}

bool XMLWriter::isOpen() const {
    return file_.is_open();
}

void XMLWriter::writeIndent() {
    for (int i = 0; i < indentLevel_; ++i) {
        file_ << "  ";
    }
}

std::string XMLWriter::escapeXML(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '&': oss << "&amp;"; break;
            case '<': oss << "&lt;"; break;
            case '>': oss << "&gt;"; break;
            case '"': oss << "&quot;"; break;
            case '\'': oss << "&apos;"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

} // namespace enigma
