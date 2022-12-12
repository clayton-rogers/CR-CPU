# CR-CPU Assembly

This is a VS Code extension which provides syntax highlighting for CR-CPU assembly. If you are using a dark theme, it's recommened to copy the following additional syntax colors into you VS Code user settings:

```json
    "editor.tokenColorCustomizations": {
        "textMateRules": [
            {
                "scope": "string.quoted.double.cr-asm",
                "settings": {
                    "foreground": "#0080FF"
                }
            },
            {
                "scope": "variable.other.register.cr-asm",
                "settings": {
                    "foreground": "#9B30FF"
                }
            },
            {
                "scope": "constant.numeric.cr-asm",
                "settings": {
                    "foreground": "#FF8C00"
                }
            },
            {
                "scope": "keyword.instruction.cr-asm",
                "settings": {
                    "foreground": "#3456ee"
                }
            },
            {
                "scope": "keyword.sp-instruction.cr-asm",
                "settings": {
                    "fontStyle": "underline",
                    "foreground": "#3456ee"
                }
            },
            {
                "scope": "storage.modifier.cr-asm",
                "settings": {
                    "foreground": "#5ecc04"
                }
            },
            {
                "scope": "storage.type.cr-asm",
                "settings": {
                    "foreground": "#0b8001"
                }
            }
        ]
    }
```

## Features

Provides grammer rules for CR-CPU assembly so that it can be syntax highlighted.

## Requirements

None.

## Extension Settings

None.

## Known Issues

None.

## Release Notes

### 0.2.3

Fix so that labels with numbers are properly highlighted.

### 0.2.2

Fix order so that conditional jumps get highlighted properly instead of just `jmp.r`.

### 0.2.0

Added language-configuration.json.

### 0.1.0

Initial useful release.

### 0.0.1

Initial dev release.

## For developers

Make sure that `npm`, `node`, and `vsce` are install. Then run:

```
vsce package
```

This will create a `.vsix` file which can then be installed.
