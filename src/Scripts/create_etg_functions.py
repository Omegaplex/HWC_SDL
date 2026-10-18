"""
create_etg_functions.py - IDAPython script to create functions at undefined code in the ETG block.

Functions in the ETG block that are only referenced via inline asm or function pointers
are missed by IDA's auto-analysis. This script scans the ETG address range for:
  - Defined code bytes (instructions) that are NOT part of any function
  - Tries to create a function starting at each such location

Run from IDA: File -> Script File -> select this file
or paste into IDA's Script Console (Shift+F2)

After running, re-check addresses in the range with list_functions via MCP to
pick up the newly created functions in the next naming pass.
"""

import idaapi
import idautils
import idc
import ida_funcs
import ida_bytes
import ida_auto

# ETG block range (inclusive)
# Extend slightly past the last known etg function to catch any trailing undefs
ETG_START = 0x468000
ETG_END   = 0x472000

def is_in_function(ea):
    return ida_funcs.get_func(ea) is not None

def try_create_function(ea):
    """
    Attempt to create a function at ea. Returns True if a new function was created.
    """
    if is_in_function(ea):
        return False
    # Make sure it looks like code (has a valid instruction)
    if not idc.create_insn(ea):
        # If create_insn fails, the bytes may not be valid x86 — skip
        return False
    # Attempt function creation
    if ida_funcs.add_func(ea):
        return True
    # Fallback: ask IDA autoanalysis to re-examine this address
    ida_auto.auto_mark_range(ea, ea + 1, idc.AU_CODE)
    return False

def scan_and_create():
    created = []
    skipped_data = []
    skipped_already = []

    ea = ETG_START
    while ea < ETG_END:
        flags = ida_bytes.get_full_flags(ea)

        if ida_bytes.is_code(flags):
            if is_in_function(ea):
                # Already part of a function — advance past it
                func = ida_funcs.get_func(ea)
                ea = func.end_ea
                skipped_already.append(ea)
                continue
            else:
                # Code bytes not in a function — try to create one here
                if try_create_function(ea):
                    fname = idc.get_func_name(ea)
                    created.append((ea, fname))
                    print(f"  [+] Created function at 0x{ea:X} -> {fname}")
                    # Advance past the newly created function
                    func = ida_funcs.get_func(ea)
                    if func:
                        ea = func.end_ea
                    else:
                        ea += 1
                else:
                    ea += 1
        elif ida_bytes.is_data(flags):
            # Data byte — skip one item
            item_size = idc.get_item_size(ea)
            skipped_data.append(ea)
            ea += max(item_size, 1)
        else:
            # Undefined byte — try to make it code, then a function
            if try_create_function(ea):
                fname = idc.get_func_name(ea)
                created.append((ea, fname))
                print(f"  [+] Created function at 0x{ea:X} (from undef) -> {fname}")
                func = ida_funcs.get_func(ea)
                if func:
                    ea = func.end_ea
                else:
                    ea += 1
            else:
                ea += 1

    return created, skipped_data

def main():
    print(f"\n=== ETG Function Creator ===")
    print(f"Scanning 0x{ETG_START:X} - 0x{ETG_END:X}")
    print()

    # Run IDA's auto-analysis on the range first to ensure disassembly is current
    print("Triggering auto-analysis pass on range...")
    ida_auto.auto_mark_range(ETG_START, ETG_END, idc.AU_USED)
    ida_auto.auto_wait()
    print("Auto-analysis done.")
    print()

    created, skipped_data = scan_and_create()

    print()
    print(f"=== Results ===")
    print(f"New functions created: {len(created)}")
    for addr, name in created:
        print(f"  0x{addr:X}  {name}")
    print(f"Data items skipped:    {len(skipped_data)}")
    print()

    if created:
        print("Save the database (Ctrl+W or File->Save) before running the naming pass.")

main()
