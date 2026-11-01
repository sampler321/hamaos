// Renders ASCII 32..126 of a monospace font into 1-bit glyph bitmaps (CW x CH) -> C header.
import Foundation
import CoreText
import CoreGraphics
let args = CommandLine.arguments
let fontName = args.count > 1 ? args[1] : "Menlo-Regular"
let size: CGFloat = args.count > 2 ? CGFloat(Double(args[2])!) : 16
let CW = args.count > 3 ? Int(args[3])! : 10, CH = args.count > 4 ? Int(args[4])! : 20
let font = CTFontCreateWithName(fontName as CFString, size, nil)
let ascent = CTFontGetAscent(font), descent = CTFontGetDescent(font)
let base = (CGFloat(CH) - (ascent + descent)) / 2 + descent
var out = "/* \(fontName) \(size)pt rendered to \(CW)x\(CH), rows of \(CW) bits (MSB left), chars 32..126 */\n"
out += "#define FONT_W \(CW)\n#define FONT_H \(CH)\nstatic const unsigned short font_bits[95][\(CH)] = {\n"
let cs = CGColorSpaceCreateDeviceGray()
for ch in 32...126 {
    let ctx = CGContext(data: nil, width: CW, height: CH, bitsPerComponent: 8, bytesPerRow: CW, space: cs, bitmapInfo: 0)!
    ctx.setFillColor(gray: 0, alpha: 1); ctx.fill(CGRect(x: 0, y: 0, width: CW, height: CH))
    ctx.setShouldAntialias(true)
    let s = String(UnicodeScalar(UInt8(ch)))
    let attr = NSAttributedString(string: s, attributes: [NSAttributedString.Key(kCTFontAttributeName as String): font,
        NSAttributedString.Key(kCTForegroundColorFromContextAttributeName as String): true])
    ctx.setFillColor(gray: 1, alpha: 1)
    let line = CTLineCreateWithAttributedString(attr)
    let w = CTLineGetTypographicBounds(line, nil, nil, nil)
    ctx.textPosition = CGPoint(x: (CGFloat(CW) - CGFloat(w)) / 2, y: base)
    CTLineDraw(line, ctx)
    let px = ctx.data!.bindMemory(to: UInt8.self, capacity: CW * CH)
    var rows: [String] = []
    for y in 0..<CH {
        var bits = 0
        for x in 0..<CW { if px[y * CW + x] > 100 { bits |= 1 << (15 - x) } }
        rows.append(String(format: "0x%04x", bits))
    }
    out += "  {" + rows.joined(separator: ",") + "}, /* '\(ch == 92 ? "\\\\" : ch == 39 ? "\\'" : s)' */\n"
}
out += "};\n"
print(out, terminator: "")
