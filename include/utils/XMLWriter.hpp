#ifndef ENIGMA_XMLWRITER_HPP
#define ENIGMA_XMLWRITER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <map>

namespace enigma {

/**
 * @brief Clase para escribir archivos XML de forma estructurada
 */
class XMLWriter {
public:
    XMLWriter(const std::string& filename);
    ~XMLWriter();

    // Control de documento
    void writeDeclaration();
    void writeComment(const std::string& comment);
    
    // Control de elementos
    void startElement(const std::string& name);
    void startElement(const std::string& name, const std::map<std::string, std::string>& attributes);
    void endElement(const std::string& name);
    void writeElement(const std::string& name, const std::string& content);
    void writeEmptyElement(const std::string& name, const std::map<std::string, std::string>& attributes);
    
    // Utilidades
    void writeRaw(const std::string& content);
    void close();
    bool isOpen() const;

private:
    std::ofstream file_;
    int indentLevel_;
    std::vector<std::string> elementStack_;
    
    void writeIndent();
    std::string escapeXML(const std::string& str);
};

} // namespace enigma

#endif // ENIGMA_XMLWRITER_HPP
