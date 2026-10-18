"""
name_etg_functions.py - IDAPython script that walks the three ETG dispatch tables
and renames all function pointers to their string-mapped names.

The ETG system has three tables:
  1. etgScriptParseTable @ 0x87AB40  {char *name(4), funcptr(4), void*(4)} = 12 bytes/entry
  2. etgFunctionCallTable             {char *name(4), ...various...} - find via xrefs
  3. etgParseTable                    {char *name(4), udword flags(4), funcptr(4)} = 12 bytes/entry

Run from IDA: File -> Script File -> select this file
After running, save the database (Ctrl+W).
"""

import idc
import idaapi
import ida_funcs
import ida_name
import ida_bytes as _ida_bytes

def _get_dword(ea):
    return _ida_bytes.get_dword(ea)

def _get_byte(ea):
    return _ida_bytes.get_byte(ea)

def read_string(ea):
    """Read a null-terminated string from a data address."""
    if ea == 0 or ea == 0xFFFFFFFF:
        return None
    try:
        s = idc.get_strlit_contents(ea, -1, idc.STRTYPE_C)
        if s:
            return s.decode('ascii', errors='replace')
    except Exception:
        pass
    # Fallback: read byte by byte
    result = []
    for i in range(256):
        b = _get_byte(ea + i)
        if b == 0:
            break
        if b < 0x20 or b > 0x7E:
            return None
        result.append(chr(b))
    return ''.join(result) if result else None

def try_rename(func_ea, new_name):
    """
    Rename a function to the table-derived new_name.
    - Always renames unnamed functions (sub_, loc_, nullsub_)
    - Always renames if current name differs from new_name (table is ground truth)
    - Skips only if already correctly named
    """
    if func_ea == 0 or func_ea == 0xFFFFFFFF:
        return False
    current = idc.get_func_name(func_ea)
    if not current:
        current = idc.get_name(func_ea)
    if not current:
        return False
    # Already has the correct name
    if current == new_name:
        print(f"  [ok] {hex(func_ea)} already correct: {current}")
        return False
    # Library-prefixed functions (won_, titan_, stl_, jpg_, crt_) — don't overwrite
    for prefix in ('won_', 'titan_', 'stl_', 'jpg_', 'crt_'):
        if current.startswith(prefix):
            print(f"  [skip] {hex(func_ea)} is library function: {current}")
            return False
    # Make sure there's a function here
    if not ida_funcs.get_func(func_ea):
        idc.create_insn(func_ea)
        ida_funcs.add_func(func_ea)
    result = idc.set_name(func_ea, new_name, idc.SN_NOWARN | idc.SN_NOCHECK)
    if result:
        print(f"  [+] {hex(func_ea)}: {current} -> {new_name}")
        return True
    else:
        print(f"  [!] Failed to rename {hex(func_ea)} to {new_name} (current: {current})")
        return False

def walk_script_entry_table(table_ea, table_name):
    """
    Walk a scriptEntry table: {char *name(4), funcptr(4), void *extra(4)} = 12 bytes/entry
    Ends at a NULL name pointer.
    Returns list of (func_ea, string_name) tuples.
    """
    print(f"\n=== {table_name} @ {hex(table_ea)} ===")
    results = []
    ea = table_ea
    while True:
        name_ptr = _get_dword(ea)
        if name_ptr == 0:
            break
        func_ea = _get_dword(ea + 4)
        name_str = read_string(name_ptr)
        if name_str is None:
            print(f"  [stop] Null/invalid string at {hex(ea)}, ptr={hex(name_ptr)}")
            break
        print(f"  entry @ {hex(ea)}: '{name_str}' -> func @ {hex(func_ea)}")
        if func_ea and func_ea != 0xFFFFFFFF:
            results.append((func_ea, name_str))
        ea += 12
        if ea > table_ea + 12 * 200:  # safety limit
            print(f"  [stop] Hit safety limit at {hex(ea)}")
            break
    return results

def walk_parse_entry_table(table_ea, table_name):
    """
    Walk an opparseentry table: {char *name(4), udword flags(4), funcptr(4)} = 12 bytes/entry
    Function pointer is at offset +8, not +4.
    Ends at a NULL name pointer.
    Returns list of (func_ea, string_name) tuples.
    """
    print(f"\n=== {table_name} @ {hex(table_ea)} ===")
    results = []
    ea = table_ea
    while True:
        name_ptr = _get_dword(ea)
        if name_ptr == 0:
            break
        func_ea = _get_dword(ea + 8)  # function is at offset +8
        name_str = read_string(name_ptr)
        if name_str is None:
            print(f"  [stop] Null/invalid string at {hex(ea)}, ptr={hex(name_ptr)}")
            break
        print(f"  entry @ {hex(ea)}: '{name_str}' -> func @ {hex(func_ea)}")
        if func_ea and func_ea != 0xFFFFFFFF:
            results.append((func_ea, name_str))
        ea += 12
        if ea > table_ea + 12 * 200:
            print(f"  [stop] Hit safety limit at {hex(ea)}")
            break
    return results

def walk_funcentry_table(table_ea, table_name):
    """
    Walk an etgvariablecallback table: 7 dwords = 28 bytes per entry.
    Cataclysm layout: {char *name(4), retType(4), nParams(4), funcptr(4), params[3](12)} = 28 bytes
    Function pointer is at offset +12. (Confirmed from etgFunctionCallScan decompile:
    function is accessed via (char*)&dword_879814 + 28*idx, where 0x879814 - 0x879808 = 0xC = 12)
    Ends at a NULL name pointer.
    """
    print(f"\n=== {table_name} @ {hex(table_ea)} ===")
    results = []
    ea = table_ea
    while True:
        name_ptr = _get_dword(ea)
        if name_ptr == 0:
            break
        func_ea = _get_dword(ea + 12)  # function at offset +12 (Cata layout: name/retType/nParams/funcPtr)
        name_str = read_string(name_ptr)
        if name_str is None:
            print(f"  [stop] Null/invalid string at {hex(ea)}, ptr={hex(name_ptr)}")
            break
        print(f"  entry @ {hex(ea)}: '{name_str}' -> func @ {hex(func_ea)}")
        if func_ea and func_ea != 0xFFFFFFFF:
            results.append((func_ea, name_str))
        ea += 28  # 7 dwords per entry
        if ea > table_ea + 28 * 300:
            print(f"  [stop] Hit safety limit at {hex(ea)}")
            break
    return results

def main():
    renamed = []
    skipped = []

    # -------------------------------------------------------------------------
    # Table 1: etgScriptParseTable @ 0x87AB40
    # Structure: {char *name, setVarCback func, void *extra} = 12 bytes
    # Known from: etgStartup calls scriptSet(dir, v0, &off_87AB40)
    # -------------------------------------------------------------------------
    ETG_SCRIPT_PARSE_TABLE = 0x87AB40
    entries1 = walk_script_entry_table(ETG_SCRIPT_PARSE_TABLE, "etgScriptParseTable")

    # Map entry names to etg* function name prefix
    # These are the property-setter callbacks called during .script file parsing
    name_map_script = {
        'LoadEffect':               'etgEffectLoad',
        'SoftwareVersion':          'etgSoftwareEffect',
        'setDeathEvent':            'etgDeathEventSet',
        'setFireEvent':             'etgFireEventSet',
        'setHitEvent':              'etgHitEventSet',
        'setBulletDestroyEvent':    'etgBulletDestroyEffectSet',
        'bigBoomFactor':            'etgSetBigDeathFactor',
        'setBulletEffect':          'etgBulletEffectSet',
        'setBulletColor':           'etgBulletColorSet',
        'setAsteroidEffect':        'etgResourceEffectSet',
        'setGaseousEffect':         'etgGaseousEffectSet',
        'setTractorBeamEffect':     'etgTractorBeamEffectSet',
        'setSpecialPurposeEffect':  'etgSpecialPurposeEffectSet',
        'setDamageEvent':           'etgDamageEventSet',
        'setHyperspaceEvent':       'etgHyperspaceEventSet',
        'NumberEffects':            'etgNumberEffectsParse',
        'etgHistoryScalarMin':      None,  # goes to scriptSetSdwordCB (generic)
        'etgSoftwareScalarDamage':  None,  # goes to scriptSetReal32CB (generic)
        'etgSoftwareScalarHit':     None,
        'etgSoftwareScalarFire':    None,
    }

    for func_ea, str_name in entries1:
        mapped = name_map_script.get(str_name)
        if mapped:
            if try_rename(func_ea, mapped):
                renamed.append((func_ea, mapped))
            else:
                skipped.append((func_ea, str_name))
        else:
            print(f"  [skip] '{str_name}' maps to generic callback, no rename needed")

    # -------------------------------------------------------------------------
    # Table 2: etgFunctionCallTable @ 0x879808
    # Structure: {char *name(4), retType(4), funcptr(4), nParams(4), params[4](16)} = 28 bytes
    # Function pointer at offset +8.
    # From: etgFunctionCallScan decompilation uses off_879808, loop stride 7 dwords (28 bytes).
    # These are the ETG built-in functions callable from .etg scripts:
    # setColor, setDrag, createSprites, damageDone, int2Color, etc.
    # -------------------------------------------------------------------------
    ETG_FUNC_TABLE = 0x879808
    entries2 = walk_funcentry_table(ETG_FUNC_TABLE, "etgFunctionCallTable")

    # Map from script-visible name to etg C function name (from HW1 ETG.c funcEntry macros)
    name_map_funcs = {
        'setEffDefaults':       'etgEffDefaults',
        'setEffDrag':           'etgEffDrag',
        'setEffOffsetDrag':     'etgEffOffsetDrag',
        'setEffOffsetTime':     'etgEffOffsetTime',
        'setEffOffsetSpin':     'etgEffOffsetSpin',
        'setEffOffsetVelLOF':   'etgEffOffsetVelLOF',
        'setEffOffsetXYZ':      'etgEffOffsetXYZ',
        'setEffOffsetR':        'etgEffOffsetR',
        'setEffChangeLOF':      'etgEffChangeLOF',
        'setEffOffsetLOF':      'etgEffOffsetLOF',
        'setEffAttachParent':   'etgEffAttachParent',
        'setEffSpin':           'etgEffSpin',
        'setEffDeltaSpin':      'etgEffDeltaSpin',
        'damageDone':           'etgDamageDone',
        'getEffectVelocity':    'etgEffectVelocityGet',
        'getColorScheme':       'etgGetColorScheme',
        'deathCries':           'etgDeathCries',
        'playChatter':          'etgPlayChatter',
        'playSound':            'etgPlaySound',
        'omitParentShipFromRenderList': 'etgOmitParentShipFromRenderList',
        'hideParentShip':       'etgParentShipHide',
        'deleteParentShip':     'etgParentShipDelete',
        'carandom':             'etgCarandom',
        'crandom':              'etgCrandom',
        'irandom':              'etgIrandom',
        'frandom':              'etgFrandom',
        'fdegsin':              'etgFdegsin',
        'fmult':                'etgFmult',
        'createLensFlares':     'etgCreateLensFlares',
        'createEBolts':         'etgCreateEBolts',
        'createMeshes':         'etgCreateMeshes',
        'createLines':          'etgCreateLines',
        'createPoints':         'etgCreatePoints',
        'createCircles':        'etgCreateCircles',
        'createSprites':        'etgCreateSprites',
        'int2Color':            'etgInts2Color',
        'int2ColorA':           'etgInts2ColorA',
        'floats2Color':         'etgFloats2Color',
        'floats2ColorA':        'etgFloats2ColorA',
        'colRed':               'etgColRed',
        'colGreen':             'etgColGreen',
        'colBlue':              'etgColBlue',
        'int2Float':            'etgInt2Float',
        'float2Int':            'etgFloat2Int',
        'effModifyDrag':        'etgEffModifyDrag',
        'effModifyOffsetTime':  'etgEffModifyOffsetTime',
        'effModifyDeltaSpin':   'etgEffModifyDeltaSpin',
        'effModifySpin':        'etgEffModifySpin',
        'effScaleVelocity':     'etgEffScaleVelocity',
        'effModifyXYZ':         'etgEffModifyXYZ',
    }
    for func_ea, str_name in entries2:
        mapped = name_map_funcs.get(str_name)
        if not mapped:
            # Derive from name: 'setColor' -> 'etgSetColor', 'modifyScale' -> 'etgModifyScale'
            mapped = 'etg' + str_name[0].upper() + str_name[1:]
        if try_rename(func_ea, mapped):
            renamed.append((func_ea, mapped))
        else:
            skipped.append((func_ea, str_name))

    # -------------------------------------------------------------------------
    # Table 3: etgParseTable @ 0x879600
    # Structure: {char *name(4), udword flags(4), funcptr(4)} = 12 bytes
    # Function pointer at offset +8.
    # From: etgOpcodeScan decompilation uses off_879600.
    # These are the ETG compile-time opcode creators: nop, at, every, label, etc.
    # -------------------------------------------------------------------------
    ETG_PARSE_TABLE = 0x879600
    entries3 = walk_parse_entry_table(ETG_PARSE_TABLE, "etgParseTable")

    # Map from keyword to compile-time parse function name (from HW1 ETG.c parseEntry macros)
    name_map_parse = {
        'nop':              'etgNOPCreate',
        'if':               'etgConditional',
        'else':             'etgElse',
        'label':            'etgNewLocalLabel',
        'eventStart':       'etgNewEventParse',
        'variable':         'etgVariableBlockOpen',
        '{':                'etgBlockOpen',
        '}':                'etgBlockClose',
        'startup':          'etgStartupBlockOpen',
        'eachFrame':        'etgEachFrameBlockOpen',
        'timeIndex':        'etgTimeIndexBlockOpen',
        'particleBlocks':   'etgNParticleBlocksSet',
        'maxFrequency':     'etgNHistorySet',
        'worldRender':      'etgWorldRenderSet',
        'int':              'etgNewInteger',
        'float':            'etgNewFloat',
        'RGB':              'etgNewRGB',
        'RGBA':             'etgNewRGBA',
        'subTextureDWORD':  'etgSubTextureDWORD',
        'morphAnimDWORD':   'etgMorphAnimDWORD',
        'setTexture':       'etgSetTextureParse',
        'modifyTexture':    'etgSetTextureParse',
        'setMesh':          'etgSetMeshParse',
        'at':               'etgAtDefine',
        'every':            'etgAtDefine',
        'before':           'etgAtDefine',
        'after':            'etgAtDefine',
        'between':          'etgBetweenDefine',
        'timeBlock':        'etgTimeBlockOpen',
        'time':             'etgTimeIndexDefine',
        'goto':             'etgLabelGoto',
        'call':             'etgLabelCall',
        'return':           'etgReturn',
        'end':              'etgEndOpcode',
        'delete':           'etgDeleteOpcode',
        'yield':            'etgYeildOpcode',
        'alternate':        'etgAternateStart',
        'alt':              'etgAternateSet',
        '#line':            'etgLineNumberUpdate',
        'spawn':            'etgSpawnParse',
        'createEffects':    'etgCreateEffectsParse',
    }
    for func_ea, str_name in entries3:
        mapped = name_map_parse.get(str_name)
        if mapped:
            if try_rename(func_ea, mapped):
                renamed.append((func_ea, mapped))
            else:
                skipped.append((func_ea, str_name))
        else:
            print(f"  [unknown] parse table entry '{str_name}' at {hex(func_ea)} - no HW1 mapping, needs manual check")

    # -------------------------------------------------------------------------
    # Report
    # -------------------------------------------------------------------------
    print(f"\n=== Summary ===")
    print(f"Renamed: {len(renamed)}")
    for ea, name in renamed:
        print(f"  {hex(ea)}  {name}")
    print(f"Skipped (already named): {len(skipped)}")
    print("\nSave the database after verifying renames (Ctrl+W)")

main()
