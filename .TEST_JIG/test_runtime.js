const fs = require('fs');
const path = require('path');

function test() {
    const htmlPath = path.join(__dirname, 'factory_tool.html');
    const html = fs.readFileSync(htmlPath, 'utf8');
    
    // Extract script contents
    const scriptRegex = /<script\b[^>]*>([\s\S]*?)<\/script>/gi;
    let match;
    let scripts = [];
    while ((match = scriptRegex.exec(html)) !== null) {
        scripts.push(match[1]);
    }
    
    console.log(`Extracted ${scripts.length} script blocks.`);
    
    // Mock basic DOM environment
    const mockElement = () => ({
        style: {},
        classList: {
            add: () => {},
            remove: () => {},
            contains: () => false
        },
        addEventListener: () => {},
        appendChild: () => {},
        querySelector: () => mockElement(),
        querySelectorAll: () => [mockElement()],
        options: [{ value: 'Guest' }, { value: 'custom' }],
        getBoundingClientRect: () => ({ width: 100 }),
        focus: () => {},
        select: () => {},
        value: '',
        textContent: '',
        innerHTML: '',
        checked: true,
        closest: () => ({ classList: { contains: () => false } })
    });
    
    global.window = {
        addEventListener: () => {},
        location: { protocol: 'http:' },
        AudioContext: function() {
            return {
                createGain: () => ({ 
                    gain: { exponentialRampToValueAtTime: () => {} },
                    connect: () => {}
                }),
                currentTime: 0
            };
        }
    };

    global.fetch = async () => ({
        ok: true,
        status: 200,
        text: async () => "",
        json: async () => ({}),
        blob: async () => ({ arrayBuffer: async () => new ArrayBuffer(0) })
    });
    
    global.document = {
        getElementById: (id) => {
            console.log(`  Querying element: ${id}`);
            return mockElement();
        },
        querySelectorAll: () => [mockElement()],
        querySelector: () => mockElement(),
        addEventListener: () => {}
    };
    
    global.localStorage = {
        getItem: () => null,
        setItem: () => {}
    };
    
    global.navigator = {
        onLine: true
    };
    
    global.setInterval = () => {};
    
    // Execute the main script block
    try {
        console.log("Running script block...");
        // Comment out the import statement for node runtime
        let code = scripts[0];
        code = code.replace(/import\s+\{\s*ESPLoader\s*,\s*Transport\s*\}\s+from\s+["'][^"']+["'];/g, 'const ESPLoader = {}, Transport = {};');
        eval(code);
        console.log("✅ Script parsed successfully! Testing autoSyncToGoogleSheets invocation...");
        eval(code);
        console.log("✅ Script parsed successfully! Testing autoSyncToGoogleSheets invocation...");
        if (typeof autoSyncToGoogleSheets === 'function') {
            sheetUrlInput.value = "https://script.google.com/macros/s/mock/exec";
            autoSyncToGoogleSheets("PASS").then((res) => {
                console.log(`✅ autoSyncToGoogleSheets executed without ReferenceErrors! Result: ${res}`);
            }).catch((err) => {
                console.error("❌ Error in autoSyncToGoogleSheets:", err);
                process.exit(1);
            });
        }
    } catch (err) {
        console.error("❌ Runtime Error during script execution:");
        console.error(err);
        process.exit(1);
    }
}

test();
