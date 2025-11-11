





//Calculate the nbr of pages needed (round to the upper)
    std::size_t num_pages = (length + 255) / 256;  // 256 bytes per page

    //If hint is null -> 0, start to search from address 0x1000
    VirtualAddress current = hint.raw == 0 ? VirtualAddress{0x1000} : hint;

    // Align the address on a page boundary -> mask the 8 least significant bits
    current = VirtualAddress{static_cast<std::uint16_t>(current.raw & ~0xff)};

    bool found = false;
    VirtualAddress start = current;

    //Search for a block of num_pages free pages
    while (!found && current.raw < 0xf800) { // Avoid the kernel's zon
      try {
        //Verified if all the pages needed are free
        found = true;
        for (std::size_t i = 0; i < num_pages && found; ++i) {
          translate(current.advanced(i * 256), 0);
          found = false; //Page already mapped
        }
      } catch (PageLookupError const& e) {
        if (e.cause == SegmentationFault) {
          //Page not mapped
          if (found) break;
          found = true;
        }
      }

      if (!found) {
        //try the next page
        current = current.advanced(256);
        //If we have looped back to the hint address -> no space available
        if (current.raw == start.raw) {
          throw std::bad_alloc();
        }
      }
    }

    if (!found || current.raw >= 0xf800) {
      throw std::bad_alloc();
    }

    //Allocate the pages
    for (std::size_t i = 0; i < num_pages; ++i) {

      allocate_page(current.advanced(i * 256), protection);
    }

    return current;