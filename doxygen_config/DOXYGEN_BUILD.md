# Doxygen + Javadoc-style comments: build integration
## doc makefile target examples
/home/asterion/3D/pick-plaz/pickplazfeederstm_ESP32Port/doxygen_config/Makefile.docs.fragment

## example Doxyfile
/home/asterion/3D/pick-plaz/pickplazfeederstm_ESP32Port/doxygen_config/Doxyfile

## Build docs locally
From project root:
```bash
make docs
```

Outputs:
- HTML: `docs/doxygen/html/index.html`
- XML:  `docs/doxygen/xml/` (useful for Sphinx+Breathe/Exhale)

## Enforcing documentation quality (CI)
This Doxyfile is configured with:

- `WARN_IF_UNDOCUMENTED = YES`
- `WARN_NO_PARAMDOC = YES`
- `WARN_AS_ERROR = FAIL_ON_WARNINGS`

So missing docs will fail the build when running:
```bash
make docs-check
```

## Javadoc-style comment conventions
Use guidance in /home/asterion/3D/pick-plaz/pickplazfeederstm_ESP32Port/doxygen_config/DOCRULES.md

Doxygen will automatically use the first sentence as the brief summary because:
- `JAVADOC_AUTOBRIEF = YES`
