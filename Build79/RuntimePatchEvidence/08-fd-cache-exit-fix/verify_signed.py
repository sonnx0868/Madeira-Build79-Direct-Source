"""Verify the exported payload, explicitly detecting the signer's input-copy bug."""
from pathlib import Path
import hashlib, json, sys, zipfile
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
sys.path.insert(0,str(HERE.parent/'guest-exit-fix'))
from build import file_offset
sha=lambda b:hashlib.sha256(b).hexdigest()
signed=ROOT/'outputs/ipad-installers/Madeira-iPad-GuestExit-v2-Test-Signed.ipa'
unsigned=ROOT/'outputs/ipad-installers/Madeira-iPad-GuestExit-v2-Test.ipa'
install_log=(HERE/'sign-install.log').read_text(errors='replace')
assert 'Installation complete!' in install_log
export_is_input_copy=sha(signed.read_bytes())==sha(unsigned.read_bytes())
with zipfile.ZipFile(unsigned) as expected, zipfile.ZipFile(signed) as actual:
    native=actual.read('Payload/Madeira.app/Madeira')
    intended=expected.read('Payload/Madeira.app/Madeira')
    # Verify the complete corrected function and the earlier abrupt-exit branch.
    for address,length in [(0x1000f8fbc,0x16c),(0x1000efd8c,24)]:
        a=file_offset(native,address,length); b=file_offset(intended,address,length)
        assert native[a:a+length]==intended[b:b+length]
    members=[]
    hashes={'ntdll.dll':'e43ae5a9bc90a02c3c01e5a451e9ab7f5570b2735b93215284868a279254c27f',
            'xtajit64.dll':'74032ec4c38a6da83e3dbe07a0a0abc62a58a823c4a193cc75dd24716239bfdb'}
    for filename,digest in hashes.items():
        matches=[n for n in expected.namelist() if n.endswith('/'+filename) and sha(expected.read(n))==digest]
        assert matches,filename
        for n in matches:
            assert sha(actual.read(n))==digest
            members.append({'member':n,'sha256':digest})
report={'installed':True,'installer_exit_code':0,'installation_complete_log_confirmed':True,
        'exported_package':str(signed),'exported_sha256':sha(signed.read_bytes()),
        'export_is_input_copy':export_is_input_copy,
        'reviewed_function_verified_in_export':True,'distinct_signed_payload_verified':not export_is_input_copy,
        'archive_caveat':'The CLI installs its signed staging bundle, but its --output exported an identical copy of the input IPA. The file named Signed.ipa is therefore not independently verified as a signed artifact. Staging was removed by the CLI.',
        'prior_guest_exit_branch_preserved':True,
        'preserved_cpu_tls_modules':members,'device_test_pending':True,'cef_fixed':False}
(HERE/'install-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
