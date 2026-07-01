import os, re

f = r'E:\000_openclaw\file\debug_tool_qt\mainwindow.cpp'
with open(f, 'r', encoding='utf-8') as fh:
    text = fh.read()

# 1. Replace all [BAD] 
text = text.replace('[BAD]', '')

# 2. Fix the orphaned settings block outside retranslateUi
pattern = r'// ==========  ==========\n\s+if \(en\) \{[\s\S]*?\n\s+\} else \{[\s\S]*?\n\s+\}'
replacement = '''    // ========== Settings Page ==========
    if (en) {
        uiSettings->groupLanguage->setTitle("Language / Language");
        uiSettings->groupAbout->setTitle("About");
        uiSettings->labelAboutVer->setText("Version: 1.0.0");
        uiSettings->labelAboutDesc->setText("FPV Debug Tool for wireless system testing");
        uiSettings->labelLangNote->setText("Restart may be required.");
    } else {
        uiSettings->groupLanguage->setTitle("Language / Language");
        uiSettings->groupAbout->setTitle("About");
        uiSettings->labelAboutVer->setText("Version: 1.0.0");
        uiSettings->labelAboutDesc->setText("FPV Debug Tool for wireless system testing");
        uiSettings->labelLangNote->setText("Restart may be required.");
    }'''

count = 0
while re.search(pattern, text):
    text = re.sub(pattern, replacement, text, count=1)
    count += 1
print('Replaced', count, 'orphaned blocks')

# 3. Fix empty group titles and labels
text = text.replace('groupLanguage->setTitle("Language / ")', 'groupLanguage->setTitle("Language / Language")')
text = text.replace('groupLanguage->setTitle(" / Language")', 'groupLanguage->setTitle("Language / Language")')
text = text.replace('groupAbout->setText("")', 'groupAbout->setTitle("About")')

# 4. Restore loadSettingsPage if missing
if 'void MainWindow::loadSettingsPage()' not in text:
    load_func = '''
void MainWindow::loadSettingsPage()
{
    m_pageSettings = new QWidget();
    uiSettings = new Ui::PageSettings();
    uiSettings->setupUi(m_pageSettings);
    uiSettings->btnLangZh->setChecked(true);
    ui->stackContent->addWidget(m_pageSettings);
}
'''
    text = text.replace(
        'void MainWindow::loadSerialPages()',
        load_func + '\nvoid MainWindow::loadSerialPages()'
    )
    print('Added loadSettingsPage function')

# 5. Ensure connect for language buttons
if 'btnLangZh' in text and 'connect(uiSettings->btnLangZh' not in text:
    text = text.replace(
        '    ui->listSidebar->setCurrentRow(0);',
        '    connect(uiSettings->btnLangZh, &QPushButton::clicked, this, &MainWindow::onSetLanguageZh);\n    connect(uiSettings->btnLangEn, &QPushButton::clicked, this, &MainWindow::onSetLanguageEn);\n\n    ui->listSidebar->setCurrentRow(0);'
    )
    print('Added language button connections')

# 6. Fix message text in onToggleConnection
text = text.replace('labelStatus->setText("  ")', 'labelStatus->setText("")')

# 7. Deduplicate adjacent lines
lines = text.split('\n')
new_lines = []
for i, l in enumerate(lines):
    if i > 0 and l == lines[i-1]:
        continue
    new_lines.append(l)
text = '\n'.join(new_lines)

# Write back
with open(f, 'w', encoding='utf-8') as fh:
    fh.write(text)

# Verify
with open(f, 'r', encoding='utf-8') as fh:
    ver = fh.read()

opens = ver.count('{')
closes = ver.count('}')
print('Braces: {=%d, }=%d, balanced=%s' % (opens, closes, 'YES' if opens == closes else 'NO (diff=%d)' % (opens - closes)))
print('Size: %d chars' % len(ver))

# Show last 20 lines for verification
lines = ver.split('\n')
print('\nLast 20 lines:')
for i in range(max(0, len(lines)-20), len(lines)):
    print('L%d: %s' % (i+1, lines[i]))
