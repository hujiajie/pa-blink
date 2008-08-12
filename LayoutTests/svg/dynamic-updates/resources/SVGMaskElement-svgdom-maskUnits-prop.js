// [Name] SVGMaskElement-svgdom-maskUnits-prop.js
// [Expected rendering result] half-opaque blue rect, with green circle in center - and a series of PASS messages

description("Tests dynamic updates of the 'maskUnits' property of the SVGMaskElement object")
createSVGTestCase();

var maskElement = createSVGElement("mask");
maskElement.setAttribute("id", "mask");
maskElement.setAttribute("x", "50");
maskElement.setAttribute("y", "50");
maskElement.setAttribute("width", "100");
maskElement.setAttribute("height", "100");
maskElement.setAttribute("maskUnits", "objectBoundingBox");

var circleElement = createSVGElement("circle");
circleElement.setAttribute("cx", "50");
circleElement.setAttribute("cy", "50");
circleElement.setAttribute("r", "50");
circleElement.setAttribute("transform", "translate(50,50)");
circleElement.setAttribute("fill", "#ffffff");
maskElement.appendChild(circleElement);

var defsElement = createSVGElement("defs");
defsElement.appendChild(maskElement);
rootSVGElement.appendChild(defsElement);

var unclippedRectElement = createSVGElement("rect");
unclippedRectElement.setAttribute("opacity", "0.5");
unclippedRectElement.setAttribute("width", "200");
unclippedRectElement.setAttribute("height", "200");
unclippedRectElement.setAttribute("fill", "blue");
rootSVGElement.appendChild(unclippedRectElement);

var rectElement = createSVGElement("rect");
rectElement.setAttribute("mask", "url(#mask)");
rectElement.setAttribute("width", "200");
rectElement.setAttribute("height", "200");
rectElement.setAttribute("fill", "green");
rootSVGElement.appendChild(rectElement);

shouldBe("maskElement.maskUnits.baseVal", "SVGUnitTypes.SVG_UNIT_TYPE_OBJECTBOUNDINGBOX");

function executeTest() {
    maskElement.maskUnits.baseVal = SVGUnitTypes.SVG_UNIT_TYPE_USERSPACEONUSE;
    shouldBe("maskElement.maskUnits.baseVal", "SVGUnitTypes.SVG_UNIT_TYPE_USERSPACEONUSE");

    completeTest();
}

startTest(rectElement, 150, 150);
