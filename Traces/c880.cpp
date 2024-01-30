#include "../cflobdd_int.h"
#include "../cflobdd_top_node_int.h"


using namespace std;
static void c880(){
// iscas85: c880

int coeff = 1;
int offset = 0;

CFLOBDD gat42 = MkProjection(coeff*0 + offset);
CFLOBDD gat75 = MkProjection(coeff*1 + offset);
CFLOBDD gat29 = MkProjection(coeff*2 + offset);
CFLOBDD gat17 = MkProjection(coeff*3 + offset);
CFLOBDD gat138 = MkProjection(coeff*4 + offset);
CFLOBDD gat1 = MkProjection(coeff*5 + offset);
CFLOBDD gat26 = MkProjection(coeff*6 + offset);
CFLOBDD gat51 = MkProjection(coeff*7 + offset);
CFLOBDD gat59 = MkProjection(coeff*8 + offset);
CFLOBDD gat156 = MkProjection(coeff*9 + offset);
CFLOBDD gat8 = MkProjection(coeff*10 + offset);
CFLOBDD gat101 = MkProjection(coeff*11 + offset);
CFLOBDD gat80 = MkProjection(coeff*12 + offset);
CFLOBDD gat268 = MkProjection(coeff*13 + offset);
CFLOBDD gat55 = MkProjection(coeff*14 + offset);
CFLOBDD gat149 = MkProjection(coeff*15 + offset);
CFLOBDD gat171 = MkProjection(coeff*16 + offset);
CFLOBDD gat106 = MkProjection(coeff*17 + offset);
CFLOBDD gat152 = MkProjection(coeff*18 + offset);
CFLOBDD gat153 = MkProjection(coeff*19 + offset);
CFLOBDD gat177 = MkProjection(coeff*20 + offset);
CFLOBDD gat143 = MkProjection(coeff*21 + offset);
CFLOBDD gat111 = MkProjection(coeff*22 + offset);
CFLOBDD gat183 = MkProjection(coeff*23 + offset);
CFLOBDD gat146 = MkProjection(coeff*24 + offset);
CFLOBDD gat116 = MkProjection(coeff*25 + offset);
CFLOBDD gat189 = MkProjection(coeff*26 + offset);
CFLOBDD gat121 = MkProjection(coeff*27 + offset);
CFLOBDD gat195 = MkProjection(coeff*28 + offset);
CFLOBDD gat126 = MkProjection(coeff*29 + offset);
CFLOBDD gat201 = MkProjection(coeff*30 + offset);
CFLOBDD gat261 = MkProjection(coeff*31 + offset);
CFLOBDD gat219 = MkProjection(coeff*32 + offset);
CFLOBDD gat96 = MkProjection(coeff*33 + offset);
CFLOBDD gat210 = MkProjection(coeff*34 + offset);
CFLOBDD gat228 = MkProjection(coeff*35 + offset);
CFLOBDD gat237 = MkProjection(coeff*36 + offset);
CFLOBDD gat246 = MkProjection(coeff*37 + offset);
CFLOBDD gat68 = MkProjection(coeff*38 + offset);
CFLOBDD gat72 = MkProjection(coeff*39 + offset);
CFLOBDD gat13 = MkProjection(coeff*40 + offset);
CFLOBDD gat73 = MkProjection(coeff*41 + offset);
CFLOBDD gat36 = MkProjection(coeff*42 + offset);
CFLOBDD gat165 = MkProjection(coeff*43 + offset);
CFLOBDD gat91 = MkProjection(coeff*44 + offset);
CFLOBDD gat159 = MkProjection(coeff*45 + offset);
CFLOBDD gat255 = MkProjection(coeff*46 + offset);
CFLOBDD gat260 = MkProjection(coeff*47 + offset);
CFLOBDD gat259 = MkProjection(coeff*48 + offset);
CFLOBDD gat267 = MkProjection(coeff*49 + offset);
CFLOBDD gat74 = MkProjection(coeff*50 + offset);
CFLOBDD gat135 = MkProjection(coeff*51 + offset);
CFLOBDD gat130 = MkProjection(coeff*52 + offset);
CFLOBDD gat207 = MkProjection(coeff*53 + offset);
CFLOBDD gat87 = MkProjection(coeff*54 + offset);
CFLOBDD gat88 = MkProjection(coeff*55 + offset);
CFLOBDD gat89 = MkProjection(coeff*56 + offset);
CFLOBDD gat90 = MkProjection(coeff*57 + offset);
CFLOBDD gat85 = MkProjection(coeff*58 + offset);
CFLOBDD gat86 = MkProjection(coeff*59 + offset);

// The circuit
cout << "gat269" << endl;
CFLOBDD gat269 = MkNand(4, gat1.root, gat8.root, gat13.root, gat17.root);
CFLOBDD gat270 = MkNand(4, gat1.root, gat26.root, gat13.root, gat17.root);
CFLOBDD gat273 = MkAnd(3, gat29.root, gat36.root, gat42.root);
CFLOBDD gat276 = MkAnd(3, gat1.root, gat26.root, gat51.root);
CFLOBDD gat279 = MkNand(4, gat1.root, gat8.root, gat51.root, gat17.root);
CFLOBDD gat280 = MkNand(4, gat1.root, gat8.root, gat13.root, gat55.root);
CFLOBDD gat284 = MkNand(4, gat59.root, gat42.root, gat68.root, gat72.root);
CFLOBDD gat285 = MkNand(gat29, gat68);
CFLOBDD gat286 = MkNand(3, gat59.root, gat68.root, gat74.root);
CFLOBDD gat287 = MkAnd(3, gat29.root, gat75.root, gat80.root);
cout << "gat290" << endl;
CFLOBDD gat290 = MkAnd(3, gat29.root, gat75.root, gat42.root);
CFLOBDD gat291 = MkAnd(3, gat29.root, gat36.root, gat80.root);
CFLOBDD gat292 = MkAnd(3, gat29.root, gat36.root, gat42.root);
CFLOBDD gat293 = MkAnd(3, gat59.root, gat75.root, gat80.root);
CFLOBDD gat294 = MkAnd(3, gat59.root, gat75.root, gat42.root);
CFLOBDD gat295 = MkAnd(3, gat59.root, gat36.root, gat80.root);
CFLOBDD gat296 = MkAnd(3, gat59.root, gat36.root, gat42.root);
CFLOBDD gat297 = MkAnd(gat85, gat86);
CFLOBDD gat298 = MkOr(gat87, gat88);
CFLOBDD gat301 = MkNand(gat91, gat96);
cout << "gat302" << endl;
CFLOBDD gat302 = MkOr(gat91, gat96);
CFLOBDD gat303 = MkNand(gat101, gat106);
CFLOBDD gat304 = MkOr(gat101, gat106);
CFLOBDD gat305 = MkNand(gat111, gat116);
CFLOBDD gat306 = MkOr(gat111, gat116);
CFLOBDD gat307 = MkNand(gat121, gat126);
CFLOBDD gat308 = MkOr(gat121, gat126);
CFLOBDD gat309 = MkAnd(gat8, gat138);
CFLOBDD gat310 = MkNot(gat268);
CFLOBDD gat316 = MkAnd(gat51, gat138);
cout << "gat317" << endl;
CFLOBDD gat317 = MkAnd(gat17, gat138);
CFLOBDD gat318 = MkAnd(gat152, gat138);
CFLOBDD gat319 = MkNand(gat59, gat156);
CFLOBDD gat322 = MkNor(gat17, gat42);
CFLOBDD gat323 = MkAnd(gat17, gat42);
CFLOBDD gat324 = MkNand(gat159, gat165);
CFLOBDD gat325 = MkOr(gat159, gat165);
CFLOBDD gat326 = MkNand(gat171, gat177);
CFLOBDD gat327 = MkOr(gat171, gat177);
CFLOBDD gat328 = MkNand(gat183, gat189);
cout << "gat329" << endl;
CFLOBDD gat329 = MkOr(gat183, gat189);
CFLOBDD gat330 = MkNand(gat195, gat201);
CFLOBDD gat331 = MkOr(gat195, gat201);
CFLOBDD gat332 = MkAnd(gat210, gat91);
CFLOBDD gat333 = MkAnd(gat210, gat96);
CFLOBDD gat334 = MkAnd(gat210, gat101);
CFLOBDD gat335 = MkAnd(gat210, gat106);
CFLOBDD gat336 = MkAnd(gat210, gat111);
CFLOBDD gat337 = MkAnd(gat255, gat259);
CFLOBDD gat338 = MkAnd(gat210, gat116);
cout << "gat339" << endl;
CFLOBDD gat339 = MkAnd(gat255, gat260);
CFLOBDD gat340 = MkAnd(gat210, gat121);
CFLOBDD gat341 = MkAnd(gat255, gat267);
CFLOBDD gat342 = MkNot(gat269);
CFLOBDD gat343 = MkNot(gat273);
CFLOBDD gat344 = MkOr(gat270, gat273);
CFLOBDD gat345 = MkNot(gat276);
CFLOBDD gat346 = MkNot(gat276);
CFLOBDD gat347 = MkNot(gat279);
CFLOBDD gat348 = MkNor(gat280, gat284);
cout << "gat349" << endl;
CFLOBDD gat349 = MkOr(gat280, gat285);
CFLOBDD gat350 = MkOr(gat280, gat286);
CFLOBDD gat351 = MkNot(gat293);
CFLOBDD gat352 = MkNot(gat294);
CFLOBDD gat353 = MkNot(gat295);
CFLOBDD gat354 = MkNot(gat296);
CFLOBDD gat355 = MkNand(gat89, gat298);
CFLOBDD gat356 = MkAnd(gat90, gat298);
CFLOBDD gat357 = MkNand(gat301, gat302);
CFLOBDD gat360 = MkNand(gat303, gat304);
cout << "gat363" << endl;
CFLOBDD gat363 = MkNand(gat305, gat306);
CFLOBDD gat366 = MkNand(gat307, gat308);
CFLOBDD gat369 = MkNot(gat310);
CFLOBDD gat375 = MkNor(gat322, gat323);
CFLOBDD gat376 = MkNand(gat324, gat325);
CFLOBDD gat379 = MkNand(gat326, gat327);
CFLOBDD gat382 = MkNand(gat328, gat329);
CFLOBDD gat385 = MkNand(gat330, gat331);
CFLOBDD gat388 = gat290;
CFLOBDD gat389 = gat291;
cout << "gat390" << endl;
CFLOBDD gat390 = gat292;
CFLOBDD gat391 = gat297;
CFLOBDD gat392 = MkOr(gat270, gat343);
CFLOBDD gat393 = MkNot(gat345);
CFLOBDD gat399 = MkNot(gat346);
CFLOBDD gat400 = MkAnd(gat348, gat73);
CFLOBDD gat401 = MkNot(gat349);
CFLOBDD gat402 = MkNot(gat350);
CFLOBDD gat403 = MkNot(gat355);
CFLOBDD gat404 = MkNot(gat357);
cout << "gat405" << endl;
CFLOBDD gat405 = MkNot(gat360);
CFLOBDD gat406 = MkAnd(gat357, gat360);
CFLOBDD gat407 = MkNot(gat363);
CFLOBDD gat408 = MkNot(gat366);
CFLOBDD gat409 = MkAnd(gat363, gat366);
CFLOBDD gat410 = MkNand(gat347, gat352);
CFLOBDD gat411 = MkNot(gat376);
CFLOBDD gat412 = MkNot(gat379);
CFLOBDD gat413 = MkAnd(gat376, gat379);
CFLOBDD gat414 = MkNot(gat382);
cout << "gat415" << endl;
CFLOBDD gat415 = MkNot(gat385);
CFLOBDD gat416 = MkAnd(gat382, gat385);
CFLOBDD gat417 = MkAnd(gat210, gat369);
CFLOBDD gat418 = gat342;
CFLOBDD gat419 = gat344;
CFLOBDD gat420 = gat351;
CFLOBDD gat421 = gat353;
CFLOBDD gat422 = gat354;
CFLOBDD gat423 = gat356;
CFLOBDD gat424 = MkNot(gat400);
cout << "gat425" << endl;
CFLOBDD gat425 = MkAnd(gat404, gat405);
CFLOBDD gat426 = MkAnd(gat407, gat408);
CFLOBDD gat427 = MkAnd(3, gat319.root, gat393.root, gat55.root);
CFLOBDD gat432 = MkAnd(3, gat393.root, gat17.root, gat287.root);
CFLOBDD gat437 = MkNand(3, gat393.root, gat287.root, gat55.root);
CFLOBDD gat442 = MkNand(4, gat375.root, gat59.root, gat156.root, gat393.root);
CFLOBDD gat443 = MkNand(3, gat393.root, gat319.root, gat17.root);
CFLOBDD gat444 = MkAnd(gat411, gat412);
CFLOBDD gat445 = MkAnd(gat414, gat415);
CFLOBDD gat446 = gat392;
cout << "gat447" << endl;
CFLOBDD gat447 = gat399;
CFLOBDD gat448 = gat401;
CFLOBDD gat449 = gat402;
CFLOBDD gat450 = gat403;
CFLOBDD gat451 = MkNot(gat424);
CFLOBDD gat460 = MkNor(gat406, gat425);
CFLOBDD gat463 = MkNor(gat409, gat426);
CFLOBDD gat466 = MkNand(gat442, gat410);
CFLOBDD gat475 = MkAnd(gat143, gat427);
CFLOBDD gat476 = MkAnd(gat310, gat432);
cout << "gat477" << endl;
CFLOBDD gat477 = MkAnd(gat146, gat427);
CFLOBDD gat478 = MkAnd(gat310, gat432);
CFLOBDD gat479 = MkAnd(gat149, gat427);
CFLOBDD gat480 = MkAnd(gat310, gat432);
CFLOBDD gat481 = MkAnd(gat153, gat427);
CFLOBDD gat482 = MkAnd(gat310, gat432);
CFLOBDD gat483 = MkNand(gat443, gat1);
CFLOBDD gat488 = MkOr(gat369, gat437);
CFLOBDD gat489 = MkOr(gat369, gat437);
CFLOBDD gat490 = MkOr(gat369, gat437);
cout << "gat491" << endl;
CFLOBDD gat491 = MkOr(gat369, gat437);
CFLOBDD gat492 = MkNor(gat413, gat444);
CFLOBDD gat495 = MkNor(gat416, gat445);
CFLOBDD gat498 = MkNand(gat130, gat460);
CFLOBDD gat499 = MkOr(gat130, gat460);
CFLOBDD gat500 = MkNand(gat463, gat135);
CFLOBDD gat501 = MkOr(gat463, gat135);
CFLOBDD gat502 = MkAnd(gat91, gat466);
CFLOBDD gat503 = MkNor(gat475, gat476);
CFLOBDD gat504 = MkAnd(gat96, gat466);
cout << "gat505" << endl;
CFLOBDD gat505 = MkNor(gat477, gat478);
CFLOBDD gat506 = MkAnd(gat101, gat466);
CFLOBDD gat507 = MkNor(gat479, gat480);
CFLOBDD gat508 = MkAnd(gat106, gat466);
CFLOBDD gat509 = MkNor(gat481, gat482);
CFLOBDD gat510 = MkAnd(gat143, gat483);
CFLOBDD gat511 = MkAnd(gat111, gat466);
CFLOBDD gat512 = MkAnd(gat146, gat483);
CFLOBDD gat513 = MkAnd(gat116, gat466);
CFLOBDD gat514 = MkAnd(gat149, gat483);
cout << "gat515" << endl;
CFLOBDD gat515 = MkAnd(gat121, gat466);
CFLOBDD gat516 = MkAnd(gat153, gat483);
CFLOBDD gat517 = MkAnd(gat126, gat466);
CFLOBDD gat518 = MkNand(gat130, gat492);
CFLOBDD gat519 = MkOr(gat130, gat492);
CFLOBDD gat520 = MkNand(gat495, gat207);
CFLOBDD gat521 = MkOr(gat495, gat207);
CFLOBDD gat522 = MkAnd(gat451, gat159);
CFLOBDD gat523 = MkAnd(gat451, gat165);
CFLOBDD gat524 = MkAnd(gat451, gat171);
cout << "gat525" << endl;
CFLOBDD gat525 = MkAnd(gat451, gat177);
CFLOBDD gat526 = MkAnd(gat451, gat183);
CFLOBDD gat527 = MkNand(gat451, gat189);
CFLOBDD gat528 = MkNand(gat451, gat195);
CFLOBDD gat529 = MkNand(gat451, gat201);
CFLOBDD gat530 = MkNand(gat498, gat499);
CFLOBDD gat533 = MkNand(gat500, gat501);
CFLOBDD gat536 = MkNor(gat309, gat502);
CFLOBDD gat537 = MkNor(gat316, gat504);
CFLOBDD gat538 = MkNor(gat317, gat506);
cout << "gat539" << endl;
CFLOBDD gat539 = MkNor(gat318, gat508);
CFLOBDD gat540 = MkNor(gat510, gat511);
CFLOBDD gat541 = MkNor(gat512, gat513);
CFLOBDD gat542 = MkNor(gat514, gat515);
CFLOBDD gat543 = MkNor(gat516, gat517);
CFLOBDD gat544 = MkNand(gat518, gat519);
CFLOBDD gat547 = MkNand(gat520, gat521);
CFLOBDD gat550 = MkNot(gat530);
CFLOBDD gat551 = MkNot(gat533);
CFLOBDD gat552 = MkAnd(gat530, gat533);
cout << "gat553" << endl;
CFLOBDD gat553 = MkNand(gat536, gat503);
CFLOBDD gat557 = MkNand(gat537, gat505);
CFLOBDD gat561 = MkNand(gat538, gat507);
CFLOBDD gat565 = MkNand(gat539, gat509);
CFLOBDD gat569 = MkNand(gat488, gat540);
CFLOBDD gat573 = MkNand(gat489, gat541);
CFLOBDD gat577 = MkNand(gat490, gat542);
CFLOBDD gat581 = MkNand(gat491, gat543);
CFLOBDD gat585 = MkNot(gat544);
CFLOBDD gat586 = MkNot(gat547);
cout << "gat587" << endl;
CFLOBDD gat587 = MkAnd(gat544, gat547);
CFLOBDD gat588 = MkAnd(gat550, gat551);
CFLOBDD gat589 = MkAnd(gat585, gat586);
CFLOBDD gat590 = MkNand(gat553, gat159);
CFLOBDD gat593 = MkOr(gat553, gat159);
CFLOBDD gat596 = MkAnd(gat246, gat553);
CFLOBDD gat597 = MkNand(gat557, gat165);
CFLOBDD gat600 = MkOr(gat557, gat165);
CFLOBDD gat605 = MkAnd(gat246, gat557);
CFLOBDD gat606 = MkNand(gat561, gat171);
cout << "gat609" << endl;
CFLOBDD gat609 = MkOr(gat561, gat171);
CFLOBDD gat615 = MkAnd(gat246, gat561);
CFLOBDD gat616 = MkNand(gat565, gat177);
CFLOBDD gat619 = MkOr(gat565, gat177);
CFLOBDD gat624 = MkAnd(gat246, gat565);
CFLOBDD gat625 = MkNand(gat569, gat183);
CFLOBDD gat628 = MkOr(gat569, gat183);
CFLOBDD gat631 = MkAnd(gat246, gat569);
CFLOBDD gat632 = MkNand(gat573, gat189);
CFLOBDD gat635 = MkOr(gat573, gat189);
cout << "gat640" << endl;
CFLOBDD gat640 = MkAnd(gat246, gat573);
CFLOBDD gat641 = MkNand(gat577, gat195);
CFLOBDD gat644 = MkOr(gat577, gat195);
CFLOBDD gat650 = MkAnd(gat246, gat577);
CFLOBDD gat651 = MkNand(gat581, gat201);
CFLOBDD gat654 = MkOr(gat581, gat201);
CFLOBDD gat659 = MkAnd(gat246, gat581);
CFLOBDD gat660 = MkNor(gat552, gat588);
CFLOBDD gat661 = MkNor(gat587, gat589);
CFLOBDD gat662 = MkNot(gat590);
cout << "gat665" << endl;
CFLOBDD gat665 = MkAnd(gat593, gat590);
CFLOBDD gat669 = MkNor(gat596, gat522);
CFLOBDD gat670 = MkNot(gat597);
CFLOBDD gat673 = MkAnd(gat600, gat597);
CFLOBDD gat677 = MkNor(gat605, gat523);
CFLOBDD gat678 = MkNot(gat606);
CFLOBDD gat682 = MkAnd(gat609, gat606);
CFLOBDD gat686 = MkNor(gat615, gat524);
CFLOBDD gat687 = MkNot(gat616);
CFLOBDD gat692 = MkAnd(gat619, gat616);
cout << "gat696" << endl;
CFLOBDD gat696 = MkNor(gat624, gat525);
CFLOBDD gat697 = MkNot(gat625);
CFLOBDD gat700 = MkAnd(gat628, gat625);
CFLOBDD gat704 = MkNor(gat631, gat526);
CFLOBDD gat705 = MkNot(gat632);
CFLOBDD gat708 = MkAnd(gat635, gat632);
CFLOBDD gat712 = MkNor(gat337, gat640);
CFLOBDD gat713 = MkNot(gat641);
CFLOBDD gat717 = MkAnd(gat644, gat641);
CFLOBDD gat721 = MkNor(gat339, gat650);
cout << "gat722" << endl;
CFLOBDD gat722 = MkNot(gat651);
CFLOBDD gat727 = MkAnd(gat654, gat651);
CFLOBDD gat731 = MkNor(gat341, gat659);
CFLOBDD gat732 = MkNand(gat654, gat261);
CFLOBDD gat733 = MkNand(3, gat644.root, gat654.root, gat261.root);
CFLOBDD gat734 = MkNand(4, gat635.root, gat644.root, gat654.root, gat261.root);
CFLOBDD gat735 = MkNot(gat662);
CFLOBDD gat736 = MkAnd(gat228, gat665);
CFLOBDD gat737 = MkAnd(gat237, gat662);
CFLOBDD gat738 = MkNot(gat670);
cout << "gat739" << endl;
CFLOBDD gat739 = MkAnd(gat228, gat673);
CFLOBDD gat740 = MkAnd(gat237, gat670);
CFLOBDD gat741 = MkNot(gat678);
CFLOBDD gat742 = MkAnd(gat228, gat682);
CFLOBDD gat743 = MkAnd(gat237, gat678);
CFLOBDD gat744 = MkNot(gat687);
CFLOBDD gat745 = MkAnd(gat228, gat692);
CFLOBDD gat746 = MkAnd(gat237, gat687);
CFLOBDD gat747 = MkNot(gat697);
CFLOBDD gat748 = MkAnd(gat228, gat700);
cout << "gat749" << endl;
CFLOBDD gat749 = MkAnd(gat237, gat697);
CFLOBDD gat750 = MkNot(gat705);
CFLOBDD gat751 = MkAnd(gat228, gat708);
CFLOBDD gat752 = MkAnd(gat237, gat705);
CFLOBDD gat753 = MkNot(gat713);
CFLOBDD gat754 = MkAnd(gat228, gat717);
CFLOBDD gat755 = MkAnd(gat237, gat713);
CFLOBDD gat756 = MkNot(gat722);
CFLOBDD gat757 = MkNor(gat727, gat261);
CFLOBDD gat758 = MkAnd(gat727, gat261);
cout << "gat759" << endl;
CFLOBDD gat759 = MkAnd(gat228, gat727);
CFLOBDD gat760 = MkAnd(gat237, gat722);
CFLOBDD gat761 = MkNand(gat644, gat722);
CFLOBDD gat762 = MkNand(gat635, gat713);
CFLOBDD gat763 = MkNand(3, gat635.root, gat644.root, gat722.root);
CFLOBDD gat764 = MkNand(gat609, gat687);
CFLOBDD gat765 = MkNand(gat600, gat678);
CFLOBDD gat766 = MkNand(3, gat600.root, gat609.root, gat687.root);
CFLOBDD gat767 = gat660;
CFLOBDD gat768 = gat661;
cout << "gat769" << endl;
CFLOBDD gat769 = MkNor(gat736, gat737);
CFLOBDD gat770 = MkNor(gat739, gat740);
CFLOBDD gat771 = MkNor(gat742, gat743);
CFLOBDD gat772 = MkNor(gat745, gat746);
CFLOBDD gat773 = MkNand(4, gat750.root, gat762.root, gat763.root, gat734.root);
CFLOBDD gat777 = MkNor(gat748, gat749);
CFLOBDD gat778 = MkNand(3, gat753.root, gat761.root, gat733.root);
CFLOBDD gat781 = MkNor(gat751, gat752);
CFLOBDD gat782 = MkNand(gat756, gat732);
CFLOBDD gat785 = MkNor(gat754, gat755);
cout << "gat786" << endl;
CFLOBDD gat786 = MkNor(gat757, gat758);
CFLOBDD gat787 = MkNor(gat759, gat760);
CFLOBDD gat788 = MkNor(gat700, gat773);
CFLOBDD gat789 = MkAnd(gat700, gat773);
CFLOBDD gat790 = MkNor(gat708, gat778);
CFLOBDD gat791 = MkAnd(gat708, gat778);
CFLOBDD gat792 = MkNor(gat717, gat782);
CFLOBDD gat793 = MkAnd(gat717, gat782);
CFLOBDD gat794 = MkAnd(gat219, gat786);
CFLOBDD gat795 = MkNand(gat628, gat773);
cout << "gat796" << endl;
CFLOBDD gat796 = MkNand(gat795, gat747);
CFLOBDD gat802 = MkNor(gat788, gat789);
CFLOBDD gat803 = MkNor(gat790, gat791);
CFLOBDD gat804 = MkNor(gat792, gat793);
CFLOBDD gat805 = MkNor(gat340, gat794);
CFLOBDD gat806 = MkNor(gat692, gat796);
CFLOBDD gat807 = MkAnd(gat692, gat796);
CFLOBDD gat808 = MkAnd(gat219, gat802);
CFLOBDD gat809 = MkAnd(gat219, gat803);
CFLOBDD gat810 = MkAnd(gat219, gat804);
cout << "gat811" << endl;
CFLOBDD gat811 = MkNand(4, gat805.root, gat787.root, gat731.root, gat529.root);
CFLOBDD gat812 = MkNand(gat619, gat796);
CFLOBDD gat813 = MkNand(3, gat609.root, gat619.root, gat796.root);
CFLOBDD gat814 = MkNand(4, gat600.root, gat609.root, gat619.root, gat796.root);
CFLOBDD gat815 = MkNand(4, gat738.root, gat765.root, gat766.root, gat814.root);
CFLOBDD gat819 = MkNand(3, gat741.root, gat764.root, gat813.root);
CFLOBDD gat822 = MkNand(gat744, gat812);
CFLOBDD gat825 = MkNor(gat806, gat807);
CFLOBDD gat826 = MkNor(gat335, gat808);
CFLOBDD gat827 = MkNor(gat336, gat809);
cout << "gat828" << endl;
CFLOBDD gat828 = MkNor(gat338, gat810);
CFLOBDD gat829 = MkNot(gat811);
CFLOBDD gat830 = MkNor(gat665, gat815);
CFLOBDD gat831 = MkAnd(gat665, gat815);
CFLOBDD gat832 = MkNor(gat673, gat819);
CFLOBDD gat833 = MkAnd(gat673, gat819);
CFLOBDD gat834 = MkNor(gat682, gat822);
CFLOBDD gat835 = MkAnd(gat682, gat822);
CFLOBDD gat836 = MkAnd(gat219, gat825);
CFLOBDD gat837 = MkNand(3, gat826.root, gat777.root, gat704.root);
cout << "gat838" << endl;
CFLOBDD gat838 = MkNand(4, gat827.root, gat781.root, gat712.root, gat527.root);
CFLOBDD gat839 = MkNand(4, gat828.root, gat785.root, gat721.root, gat528.root);
CFLOBDD gat840 = MkNot(gat829);
CFLOBDD gat841 = MkNand(gat815, gat593);
CFLOBDD gat842 = MkNor(gat830, gat831);
CFLOBDD gat843 = MkNor(gat832, gat833);
CFLOBDD gat844 = MkNor(gat834, gat835);
CFLOBDD gat845 = MkNor(gat334, gat836);
CFLOBDD gat846 = MkNot(gat837);
CFLOBDD gat847 = MkNot(gat838);
cout << "gat848" << endl;
CFLOBDD gat848 = MkNot(gat839);
CFLOBDD gat849 = MkAnd(gat735, gat841);
CFLOBDD gat850 = gat840;
CFLOBDD gat851 = MkAnd(gat219, gat842);
CFLOBDD gat852 = MkAnd(gat219, gat843);
CFLOBDD gat853 = MkAnd(gat219, gat844);
CFLOBDD gat854 = MkNand(3, gat845.root, gat772.root, gat696.root);
CFLOBDD gat855 = MkNot(gat846);
CFLOBDD gat856 = MkNot(gat847);
CFLOBDD gat857 = MkNot(gat848);
cout << "gat858" << endl;
CFLOBDD gat858 = MkNot(gat849);
CFLOBDD gat859 = MkNor(gat417, gat851);
CFLOBDD gat860 = MkNor(gat332, gat852);
CFLOBDD gat861 = MkNor(gat333, gat853);
CFLOBDD gat862 = MkNot(gat854);
CFLOBDD gat863 = gat855;
CFLOBDD gat864 = gat856;
CFLOBDD gat865 = gat857;
CFLOBDD gat866 = gat858;
CFLOBDD gat867 = MkNand(3, gat859.root, gat769.root, gat669.root);
cout << "gat868" << endl;
CFLOBDD gat868 = MkNand(3, gat860.root, gat770.root, gat677.root);
CFLOBDD gat869 = MkNand(3, gat861.root, gat771.root, gat686.root);
CFLOBDD gat870 = MkNot(gat862);
CFLOBDD gat871 = MkNot(gat867);
CFLOBDD gat872 = MkNot(gat868);
CFLOBDD gat873 = MkNot(gat869);
CFLOBDD gat874 = gat870;
CFLOBDD gat875 = MkNot(gat871);
CFLOBDD gat876 = MkNot(gat872);
CFLOBDD gat877 = MkNot(gat873);
cout << "gat878" << endl;
CFLOBDD gat878 = gat875;
CFLOBDD gat879 = gat876;
CFLOBDD gat880 = gat877;

unsigned int nodeCount, edgeCount;
cout << "CFLOBDD sizes" << endl;
GroupCountNodesAndEdgesStart(nodeCount, edgeCount);
gat388.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat389.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat390.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat391.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat418.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat419.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat420.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat421.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat422.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat423.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat446.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat447.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat448.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat449.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat450.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat767.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat768.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat850.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat863.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat864.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat865.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat866.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat874.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat878.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat879.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat880.GroupCountNodesAndEdges(nodeCount, edgeCount);
GroupCountNodesAndEdgesEnd();
cout << nodeCount << ", " << edgeCount << endl;

/*
GroupDumpConnectionsStart();
gat388.GroupDumpConnections(cout);
gat389.GroupDumpConnections(cout);
gat390.GroupDumpConnections(cout);
gat391.GroupDumpConnections(cout);
gat418.GroupDumpConnections(cout);
gat419.GroupDumpConnections(cout);
gat420.GroupDumpConnections(cout);
gat421.GroupDumpConnections(cout);
gat422.GroupDumpConnections(cout);
gat423.GroupDumpConnections(cout);
gat446.GroupDumpConnections(cout);
gat447.GroupDumpConnections(cout);
gat448.GroupDumpConnections(cout);
gat449.GroupDumpConnections(cout);
gat450.GroupDumpConnections(cout);
gat767.GroupDumpConnections(cout);
gat768.GroupDumpConnections(cout);
gat850.GroupDumpConnections(cout);
gat863.GroupDumpConnections(cout);
gat864.GroupDumpConnections(cout);
gat865.GroupDumpConnections(cout);
gat866.GroupDumpConnections(cout);
gat874.GroupDumpConnections(cout);
gat878.GroupDumpConnections(cout);
gat879.GroupDumpConnections(cout);
gat880.GroupDumpConnections(cout);
GroupDumpConnectionsEnd();
*/

/*
cout << "----------------------------" << endl;
cout << gat388 << endl;
cout << "----------------------------" << endl;
cout << gat389 << endl;
cout << "----------------------------" << endl;
cout << gat390 << endl;
cout << "----------------------------" << endl;
cout << gat391 << endl;
cout << "----------------------------" << endl;
cout << gat418 << endl;
cout << "----------------------------" << endl;
cout << gat419 << endl;
cout << "----------------------------" << endl;
cout << gat420 << endl;
cout << "----------------------------" << endl;
cout << gat421 << endl;
cout << "----------------------------" << endl;
cout << gat422 << endl;
cout << "----------------------------" << endl;
cout << gat423 << endl;
cout << "----------------------------" << endl;
cout << gat446 << endl;
cout << "----------------------------" << endl;
cout << gat447 << endl;
cout << "----------------------------" << endl;
cout << gat448 << endl;
cout << "----------------------------" << endl;
cout << gat449 << endl;
cout << "----------------------------" << endl;
cout << gat450 << endl;
cout << "----------------------------" << endl;
cout << gat767 << endl;
cout << "----------------------------" << endl;
cout << gat768 << endl;
cout << "----------------------------" << endl;
cout << gat850 << endl;
cout << "----------------------------" << endl;
cout << gat863 << endl;
cout << "----------------------------" << endl;
cout << gat864 << endl;
cout << "----------------------------" << endl;
cout << gat865 << endl;
cout << "----------------------------" << endl;
cout << gat866 << endl;
cout << "----------------------------" << endl;
cout << gat874 << endl;
cout << "----------------------------" << endl;
cout << gat878 << endl;
cout << "----------------------------" << endl;
cout << gat879 << endl;
cout << "----------------------------" << endl;
cout << gat880 << endl;
*/
}
