/**
 * Generates a PDF from the nodejs rendered html with css
 */

const puppeteer = require('puppeteer');
const path = require('path');

(async () => {
  const filePath = path.resolve(__dirname, 'index.html');
  const fileUrl = `file://${filePath}`;
  const browser = await puppeteer.launch();
  const page = await browser.newPage();
  await page.goto(fileUrl, { waitUntil: 'networkidle0' });
  await page.pdf({ path: 'roadmap.pdf', format: 'A4' });
  await browser.close();
})();