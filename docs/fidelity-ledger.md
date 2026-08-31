# Digital Field Guide Fidelity Ledger

The digital manual was shaped from the approved Dusk Cartography direction:
an offset-print field guide with paper stock, deep blue ink, oxidized teal,
ember orange, compact mono labels, and readable Genesis-era pixel art.

| Reference point | Implementation decision | Acceptance evidence |
| --- | --- | --- |
| Paper archive / cartridge insert | Warm paper texture, offset lines, oversized serif display type, and section numbering | Desktop render inspected at 1440x1800 |
| Deep blue and ember palette | Sticky blue topbar, orange action buttons, teal controls panel, and gold highlights | Desktop and mobile renders inspected |
| Cinematic dusk scene | Translated into a dark Stonewake map panel using the actual public-authored hero, dragon, and lantern-mark assets | Hero artwork loaded in the latest render |
| Genesis-era pixel language | Indexed, transparent sprites with nearest-neighbor display and a deliberately limited visual vocabulary | `prep_assets.py` output and SGDK resource compilation pass |
| Field-guide readability | Spoiler-aware route section, high-contrast controls, and a print counterpart with page breaks | Spoiler toggle interaction and PDF export pass |
| Small-screen access | Stacked hero actions, single-column sections, and mobile navigation | 390px viewport reported no horizontal overflow |

The cinematic scene is intentionally adapted rather than copied literally: the
digital guide points back to the real Stonewake route and real cartridge
assets, so its art direction remains connected to the playable project.
