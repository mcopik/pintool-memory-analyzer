
#include <iomanip>
#include <cstdio>

#include "region.hpp"

void Region::print(std::ofstream & of)
{
  of << "#region " << _region_name << " " << _counts.size() << '\n';
  for(iter_t it = _counts.begin(); it != _counts.end(); ++it) {
    of << std::hex << (*it).first.first << std::dec << " " << (*it).first.second << " ";
    of << (*it).second.read_count << " " << (*it).second.write_count << " ";
    of << (*it).second.read_count_outside << " " << (*it).second.write_count_outside << '\n';
  }
}

uintptr_t Region::align_address(uintptr_t addr)
{
  if(addr % _cacheline_size)
    return addr - addr % _cacheline_size;
  else
    return addr;
}

void Region::read(uintptr_t addr, int32_t size)
{
  addr = align_address(addr);

  iter_t it = _counts.find(std::make_pair(addr, size));

  if(it == _counts.end()) {

    _counts.insert(std::make_pair(std::make_pair(addr, size), AccessStats{1, 0, 0, 0}));

  } else {

    (*it).second.read_count += 1;

  }

}

void Region::read_host(uintptr_t addr, int32_t size)
{
  addr = align_address(addr);

  iter_t it = _counts.find(std::make_pair(addr, size));

  if(it != _counts.end()) {

    (*it).second.read_count_outside += 1;

  }

}

void Region::write(uintptr_t addr, int32_t size)
{
  addr = align_address(addr);
  iter_t it = _counts.find(std::make_pair(addr, size));

  if(it == _counts.end()) {
    _counts.insert(std::make_pair(std::make_pair(addr, size), AccessStats{0, 1, 0, 0}));
  } else {
    (*it).second.write_count += 1;
  }
}

void Region::write_host(uintptr_t addr, int32_t size)
{
  addr = align_address(addr);
  iter_t it = _counts.find(std::make_pair(addr, size));

  if(it != _counts.end()) {
    (*it).second.write_count_outside += 1;
  }
}

void Region::reset()
{
  _counts.clear();
  _count++;
}

Regions::~Regions()
{
  this->close();
}

void Regions::filename(const std::string& file_name)
{
  _file_name = file_name;
}

void Regions::close()
{
  // Ignore data from unfinished regions
  log_file.close();
  for(iter_t it = _regions.begin(); it != _regions.end(); ++it)
    delete (*it).second;
}

Region* Regions::startRegion(std::string name, int cacheline_size)
{
  iter_t it = _regions.find(name);

  if(it  == _regions.end()) {

    Region* region = new Region{name, cacheline_size};
    _regions.insert(std::make_pair(name, region));
    return region;

  } else {
    return (*it).second;
  }

}

void Regions::endRegion(Region* region)
{
  char counter[17];
  sprintf(counter, "%d", region->_count);
  std::string file = _file_name+ "." + region->_region_name + "." + counter;
  log_file.open(file.c_str(), std::ios::out);
  region->print(log_file);
  region->reset();
  log_file.close();
}


