/*
 * Maxime Moge dda119141@gmail.com 05.07.2021
*/

#ifndef STATICMAPANALYZER_H
#define STATICMAPANALYZER_H

#include <string>
#include <utility>
#include <unordered_map>
#include "json.hpp"
#include <sstream>
#include <fstream>
#include <regex>

using json = nlohmann::json;

namespace mapAnalyser {
	static const std::vector<std::string> core_descritption = {
		" -- core 0 brief description\t ********************************************",
		" -- core 1 brief description\t ****************************",
		" -- core 2 brief description\t ********************************************",
		" -- core 3 brief description\t ****************************"
	};
}

unsigned long def_to_decimal(const std::string& val)
{
	return std::strtoul(val.c_str(), NULL, 0);
}

const auto make_KB_str(unsigned int val)
{
	std::stringstream str_obj;

	str_obj << val / 1024 << " KB";

	return str_obj.str();
}

const auto make_str_FromKBytes(unsigned int val)
{
	std::stringstream str_obj;

	str_obj << val << " KB";

	return str_obj.str();
}

constexpr unsigned int make_KB(unsigned int val)
{
	return val * 1024;
}

constexpr unsigned int make_percent(unsigned int numerator, unsigned int denominator)
{
	return static_cast<unsigned int>(static_cast<double>(numerator) / denominator * 100);
}

namespace parser
{
	template <class T>
	inline bool value_not_in_vector(const std::vector<T>& container, const T& value)
	{
		return std::none_of(std::begin(container), std::end(container), [&value](const T& container_value) {
			return container_value == value;
			});
	}

	inline bool check_first_str(const std::string& subject, const std::string& substring)
	{
		if (subject.find(substring) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	class entry_list {
	private:
		std::regex words_regex = {}; 
		std::sregex_iterator words_begin = {};
		std::sregex_iterator words_end = {};
		decltype(std::distance(words_begin, words_end)) entries_found = 0;

	public:
		entry_list(const std::string& parse_subject) :
			words_regex("(\\S+)")
			, words_begin(std::sregex_iterator(parse_subject.begin(),
				parse_subject.end(), words_regex))
			, words_end(std::sregex_iterator())
			, entries_found(std::distance(words_begin, words_end))
		{
		}

		std::string find_entry(unsigned int position) const
		{
			const auto find_ent = [this](unsigned int position) -> std::string
			{
				unsigned int index = 0;

				auto words_begin_ = this->words_begin;

				while (index < this->entries_found) {
					std::smatch match = *words_begin_++;
					if (index == position) {
						return match.str();
					}
					index = index + 1;
				}

				//position parameter > number of entries found
				//throw std::out_of_range("position out of range");
				return std::string("");
			};

			return find_ent(position);
		}
	};

	int find_numeric_entry(const std::string& parse_subject, unsigned int position)
	{
		//const auto entry_str = find_entry(parse_subject, position);
		const auto entry_str = entry_list(parse_subject).find_entry(position);

		if (entry_str == std::string("")) {
			return -1;
		}
		else if (!std::isxdigit(entry_str[0]) && 
			!std::isxdigit(entry_str[1])
			&& !std::isxdigit(entry_str[2])) {

			return -1;
		}
		else {
			if (!check_first_str(entry_str, "0x") || !check_first_str(entry_str, "0X")) {

				const auto entry_str_new = std::string("0x") + entry_str;
				return def_to_decimal(entry_str_new);
			}
			else {
				return def_to_decimal(entry_str);
			}
		}
	}

	constexpr bool check_address_range(uint32_t address, uint32_t start_address, uint32_t end_address)
	{
		if ( (address < start_address) || (address > end_address) )
			return false;
		else
			return true;
	}

	unsigned int get_substring(const std::string& subject)
	{
		const auto pos = subject.find(" ");

		return stoi(subject.substr(0, pos));
	}

}

class coreUnit;

struct Section_by_memory_type
{
	unsigned int used_memory = 0;
	unsigned int start_section;
	unsigned int end_section;
	unsigned int used_memory_position = 0;
	std::vector<unsigned int> retrieved_section_entries;
};

struct ram_section
{
	unsigned int used_memory = 0;
	std::vector<unsigned int> retrieved_section_entries;
};

class MemSectionPattern
{
	unsigned int m_core;

	friend class coreUnit;
	MemSectionPattern() = default;

public:

	std::unordered_map<std::string, struct ram_section> ram_sections;

	explicit MemSectionPattern(unsigned int core): m_core(core)
	{
	}
};


class coreUnit
{
public:
	coreUnit(unsigned int core):
		m_core(core),
		m_memory_sections(core)
	{
		const auto jfile = std::make_unique<std::ifstream>("memory_map.json");

		*jfile >> ram_sections_from_json;	
	}

	/* retrieve ram section used memory and store it in m_memory_sections */
	void update_memory_section_load(const std::string& line)
	{
		const unsigned int section_size_position = ram_sections_from_json["section_size_position"];

		for (const auto& sections : ram_sections_from_json["core"]) {
			const unsigned int core_id = sections["core_id"];

			if (core_id != m_core) {
				continue;
			}

			for (const auto& area_by_memory_type : sections["memory"]) {

				const auto software_section = std::string(area_by_memory_type["memory_id"]);
				const auto retrieved_address = parser::find_numeric_entry(line, 1);

				if (retrieved_address == -1) {
					continue;
				}

				const auto& section_name = software_section.c_str();

				//retrieve known memory section
				if (m_memory_sections.ram_sections.find(section_name) == m_memory_sections.ram_sections.end()) {
					m_memory_sections.ram_sections.insert({section_name, { 0 } });
				}

				const auto start_section = def_to_decimal(std::string(area_by_memory_type["start_section"]));
				const auto end_section = def_to_decimal(std::string(area_by_memory_type["end_section"]));

				auto& ram_section = m_memory_sections.ram_sections[section_name];
				const auto& cached_entries = ram_section.retrieved_section_entries;
				const auto used_bytes_from_line = parser::find_numeric_entry(line, section_size_position);

				if (parser::value_not_in_vector<unsigned int>(cached_entries, 
					retrieved_address + used_bytes_from_line)) {

					if (parser::check_address_range(retrieved_address,
						start_section, end_section)) {

						ram_section.used_memory +=	used_bytes_from_line;

						ram_section.retrieved_section_entries.push_back(retrieved_address + used_bytes_from_line);

						break;
					}
				}
			}
		}
	}

	void update_memory_usage(std::ofstream& output_file)
	{
		for (auto& sections : ram_sections_from_json["core"]) {
			const unsigned int core_id = sections["core_id"];

			if (core_id != m_core) {
				continue;
			}

			output_file << "\n\n ***************************** Core " << m_core;

			if (mapAnalyser::core_descritption.size() >= m_core) {
				output_file << mapAnalyser::core_descritption[m_core];
			}

			print_output_headings(output_file);

			for (const auto& softwareSection : sections["memory"]) {

				const auto software_section_id = std::string(softwareSection["memory_id"]);
				const auto memory_short_id = std::string(softwareSection["short_id"]);

				const auto max_size = parser::get_substring(std::string(softwareSection["max_size"])) * 1024;

				const auto& used_memory = m_memory_sections.ram_sections[software_section_id].used_memory;

				output_file.width(25);  output_file << software_section_id;
				output_file.width(25); output_file << memory_short_id;
				output_file.width(6);  output_file << " size: ";
				output_file.width(7);  output_file << make_KB_str(used_memory);
				output_file.width(18);  output_file << " out of maximum: ";
				output_file.width(7);  output_file << make_KB_str(max_size);
				output_file << std::endl;

				if (max_size > 0) {
					output_file << "Used space in percentage: " << make_percent(used_memory, max_size) << " %";
					output_file << std::endl; output_file << std::endl;
				}
			}
		}
			
	}

private:
	unsigned int m_core;
	MemSectionPattern m_memory_sections;
	json ram_sections_from_json;

	void print_output_headings(std::ofstream& output_file)
	{
		output_file << std::endl;
		output_file.width(22); output_file << std::left << "Memory type";
		output_file.width(16); output_file << std::left << " size";
		output_file << std::endl;
	}

	//disable copying
	coreUnit(coreUnit&&) = delete;
	coreUnit(const coreUnit&) = delete;
};


class lineParser
{

public:

	lineParser():
		output_file("Memory_Analysis_Result.txt", std::ios::out | std::ios::trunc)
	{
		{
			const auto jfile = std::make_unique<std::ifstream>("memory_map.json");

			json ram_sections_from_json;

			*jfile >> ram_sections_from_json;		

			for (const auto& filename_str : ram_sections_from_json["input_file_names"]) {
				const auto filename = std::string(filename_str);

				input_files.push_back(std::make_unique<std::ifstream>(filename));
				input_file_names.push_back(filename);
			}

			max_lines_per_file = ram_sections_from_json["max_lines_per_file"];
		}

		for (unsigned int i = 0; i < 4; ++i) {
			m_cores.push_back(std::make_unique<coreUnit>(i));
		}

		parseSize();
	}

	bool update_total_values()
	{
		output_file << "\n-- ---------- Developped by Maxime Moge - dda119141@gmail.com ----------------\n";
		output_file << "---- ---------- ---------------------------------------------------- ----------------\n";
		output_file << "---- ---------- ---------------------------------------------------- ----------------\n";
		output_file << "---- ---------- ---------------------------------------------------- ----------------\n";
		output_file << "---- ---------- ---------------------------------------------------- ----------------\n";
		output_file << "\n ";

		for (const auto& coreUnitObj : m_cores)
		{
			coreUnitObj->update_memory_usage(output_file);
		}

		return true;
	}
private:
	std::vector<std::unique_ptr<coreUnit>> m_cores;
	std::vector< std::unique_ptr<std::ifstream>> input_files;
	std::vector<std::string> input_file_names;
	std::string line;
	std::ofstream output_file;
	unsigned int max_lines_per_file = 0;
	unsigned int parsedLineCounter = 0;
	unsigned int parsedFileCounter = 0;

	//disable copying
	lineParser(lineParser&&) = delete;
	lineParser(const lineParser&) = delete;

	const uint32_t parseSize()
	{
		for (auto& input_file : input_files) {

			bool Jump_to_next_file = false;
			std::cout << "file: " << input_file_names[parsedFileCounter++] << std::endl;

			while (std::getline(*input_file, line) && !Jump_to_next_file) {
				if (!line.empty()) {

					std::cout << "counter: " << parsedLineCounter++ << std::endl;

					if (parsedLineCounter > max_lines_per_file) {
						parsedLineCounter = 0;
						Jump_to_next_file = true;
					}

					for (auto& coreUnitObj : m_cores)
					{
						const std::string line_input = line;

						coreUnitObj->update_memory_section_load(line_input);
					}
				}
			}
		}

		return 0;
	}

};


#endif // !STATICMAPANALYZER_H
