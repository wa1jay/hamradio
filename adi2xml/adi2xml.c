#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024

/**
 * @brief Escapes special XML characters in a string.
 *
 * Replaces characters like &, <, >, ", and ' with their corresponding
 * XML entity references.
 *
 * @param src The input string to escape.
 * @param dest The destination buffer where the escaped string is written.
 */
void escape_xml(const char *src, char *dest) {
    while (*src) {
        switch (*src) {
            case '&': strcat(dest, "&amp;"); break;
            case '<': strcat(dest, "&lt;"); break;
            case '>': strcat(dest, "&gt;"); break;
            case '"': strcat(dest, "&quot;"); break;
            case '\'': strcat(dest, "&apos;"); break;
            default: strncat(dest, src, 1); break;
        }
        src++;
    }
}

/**
 * @brief Parses an ADIF field and writes it as an XML element.
 *
 * Extracts the tag name and field value, escapes the value for XML,
 * and writes the formatted tag to the output file.
 *
 * @param field The ADIF field string (e.g., "<CALL:5>W1AW").
 * @param xml_out The file pointer to the XML output file.
 * @param indent_level Number of indentation levels for formatting.
 * @param is_header Flag indicating if this field is part of the header.
 */
void parse_field(const char *field, FILE *xml_out, int indent_level, int is_header) {
    const char *tag_start = field + 1;
    const char *colon = strchr(tag_start, ':');
    if (!colon) return;

    char tag[64] = {0};
    strncpy(tag, tag_start, colon - tag_start);

    int length = atoi(colon + 1);
    const char *value_start = strchr(colon, '>') + 1;

    if (!value_start || length <= 0) return;

    char value[256] = {0};
    strncpy(value, value_start, length);

    char xml_safe[512] = {0};
    escape_xml(value, xml_safe);

    for (int i = 0; i < indent_level; i++) fprintf(xml_out, "  ");
    fprintf(xml_out, "<%s>%s</%s>\n", tag, xml_safe, tag);
}

/**
 * @brief Processes a line from the ADIF header and converts its fields to XML.
 *
 * Parses all ADIF fields found in the line and writes them to the XML output.
 *
 * @param line A line from the ADIF file header.
 * @param xml_out The file pointer to the XML output file.
 * @param indent_level Number of indentation levels for formatting.
 */
void process_header_line(const char *line, FILE *xml_out, int indent_level) {
    const char *pos = line;
    while ((pos = strchr(pos, '<'))) {
        const char *end = strchr(pos, '>');
        if (!end) break;

        char field[128] = {0};
        int len = end - pos + 1;
        strncpy(field, pos, len);
        strcat(field, end + 1);

        parse_field(field, xml_out, indent_level, 1);
        pos = end + 1;
    }
}

/**
 * @brief Processes a line from the ADIF QSO data section and writes XML.
 *
 * Handles creation and closure of <QSO> tags and processes fields within each QSO.
 *
 * @param line A line from the ADIF file containing QSO data.
 * @param xml_out The file pointer to the XML output file.
 * @param in_qso Pointer to a flag indicating whether we are inside a <QSO> block.
 * @param indent_level Number of indentation levels for formatting.
 */
void process_qso_line(const char *line, FILE *xml_out, int *in_qso, int indent_level) {
    const char *pos = line;
    while ((pos = strchr(pos, '<'))) {
        if (strstr(pos, "<eor>") || strstr(pos, "<EOR>")) {
            if (*in_qso) {
                for (int i = 0; i < indent_level - 1; i++) fprintf(xml_out, "  ");
                fprintf(xml_out, "</QSO>\n");
                *in_qso = 0;
            }
            pos += 5;
            continue;
        }

        const char *end = strchr(pos, '>');
        if (!end) break;

        char field[128] = {0};
        int len = end - pos + 1;
        strncpy(field, pos, len);
        strcat(field, end + 1);

        if (!*in_qso) {
            for (int i = 0; i < indent_level - 1; i++) fprintf(xml_out, "  ");
            fprintf(xml_out, "<QSO>\n");
            *in_qso = 1;
        }

        parse_field(field, xml_out, indent_level, 0);
        pos = end + 1;
    }
}

/**
 * @brief Main function that reads an ADIF file and converts it to XML format.
 *
 * Usage: ./program input.adi output.xml
 *
 * Opens the input and output files, processes the header and QSO sections,
 * and writes structured XML output.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit status (0 for success, 1 for error).
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input.adi output.xml\n", argv[0]);
        return 1;
    }

    FILE *adi = fopen(argv[1], "r");
    FILE *xml = fopen(argv[2], "w");
    if (!adi || !xml) {
        perror("File error");
        return 1;
    }

    char line[MAX_LINE];
    int in_header = 1;
    int in_qso = 0;

    fprintf(xml, "<ADI>\n");
    fprintf(xml, "  <HEADER>\n");
    
    // Process header
    while (fgets(line, MAX_LINE, adi)) {
        if (strchr(line, '<') && (strstr(line, "<eoh>") || strstr(line, "<EOH>"))) {
            fprintf(xml, "  </HEADER>\n");
            fprintf(xml, "  <QSOS>\n");
            in_header = 0;
            break;
        }
        process_header_line(line, xml, 2);
    }

    // Process QSOs
    while (fgets(line, MAX_LINE, adi)) {
        process_qso_line(line, xml, &in_qso, 3);
    }

    if (in_qso) {
        fprintf(xml, "    </QSO>\n");
    }

    fprintf(xml, "  </QSOS>\n");
    fprintf(xml, "</ADI>\n");

    fclose(adi);
    fclose(xml);

    printf("Conversion complete. Output written to %s\n", argv[2]);
    return 0;
}
