data 'TMPL' (128, "LPic") {
	$"1344 6566 6175 6C74 204C 616E 6775 6167"            /* .Default Languag */
	$"6520 4944 4457 5244 0543 6F75 6E74 4F43"            /* e IDDWRD.CountOC */
	$"4E54 042A 2A2A 2A4C 5354 430B 7379 7320"            /* NT.****LSTC.sys  */
	$"6C61 6E67 2049 4444 5752 441E 6C6F 6361"            /* lang IDDWRD.loca */
	$"6C20 7265 7320 4944 2028 6F66 6673 6574"            /* l res ID (offset */
	$"2066 726F 6D20 3530 3030 4457 5244 1032"            /*  from 5000DWRD.2 */
	$"2D62 7974 6520 6C61 6E67 7561 6765 3F44"            /* -byte language?D */
	$"5752 4404 2A2A 2A2A 4C53 5445"                      /* WRD.****LSTE */
};

data 'LPic' (5000) {
	$"0000 0002 0000 0000 0000 0001 0004 0000"            /* ................ */
                                       - Default language code (ID)
                                  - Set to 1 to display language drop down
};

data 'STR#' (5000, "English buttons") {
	$"0006 0D45 6E67 6C69 7368 2074 6573 7431"            /* ...English test1 */
	$"0541 6772 6565 0844 6973 6167 7265 6505"            /* .Agree.Disagree. */
	$"5072 696E 7407 5361 7665 2E2E 2E7A 4966"            /* Print.Save...zIf */
	$"2079 6F75 2061 6772 6565 2077 6974 6820"            /*  you agree with  */
	$"7468 6520 7465 726D 7320 6F66 2074 6869"            /* the terms of thi */
	$"7320 6C69 6365 6E73 652C 2063 6C69 636B"            /* s license, click */
	$"2022 4167 7265 6522 2074 6F20 6163 6365"            /*  "Agree" to acce */
	$"7373 2074 6865 2073 6F66 7477 6172 652E"            /* ss the software. */
	$"2020 4966 2079 6F75 2064 6F20 6E6F 7420"            /*   If you do not  */
	$"6167 7265 652C 2070 7265 7373 2022 4469"            /* agree, press "Di */
	$"7361 6772 6565 2E22"                                /* sagree." */
};

data 'STR#' (5001, "German") {
	$"0006 0744 6575 7473 6368 0B41 6B7A 6570"            /* ...Deutsch.Akzep */
	$"7469 6572 656E 0841 626C 6568 6E65 6E07"            /* tieren.Ablehnen. */
	$"4472 7563 6B65 6E0A 5369 6368 6572 6E2E"            /* Drucken¬Sichern. */
	$"2E2E E74B 6C69 636B 656E 2053 6965 2069"            /* ..ÁKlicken Sie i */
	$"6E20 D241 6B7A 6570 7469 6572 656E D32C"            /* n “Akzeptieren”, */
	$"2077 656E 6E20 5369 6520 6D69 7420 6465"            /*  wenn Sie mit de */
	$"6E20 4265 7374 696D 6D75 6E67 656E 2064"            /* n Bestimmungen d */
	$"6573 2053 6F66 7477 6172 652D 4C69 7A65"            /* es Software-Lize */
	$"6E7A 7665 7274 7261 6773 2065 696E 7665"            /* nzvertrags einve */
	$"7273 7461 6E64 656E 2073 696E 642E 2046"            /* rstanden sind. F */
	$"616C 6C73 206E 6963 6874 2C20 6269 7474"            /* alls nicht, bitt */
	$"6520 D241 626C 6568 6E65 6ED3 2061 6E6B"            /* e “Ablehnen” ank */
	$"6C69 636B 656E 2E20 5369 6520 6B9A 6E6E"            /* licken. Sie könn */
	$"656E 2064 6965 2053 6F66 7477 6172 6520"            /* en die Software  */
	$"6E75 7220 696E 7374 616C 6C69 6572 656E"            /* nur installieren */
	$"2C20 7765 6E6E 2053 6965 20D2 416B 7A65"            /* , wenn Sie “Akze */
	$"7074 6965 7265 6ED3 2061 6E67 656B 6C69"            /* ptieren” angekli */
	$"636B 7420 6861 6265 6E2E"                           /* ckt haben. */
};

data 'STR#' (5002, "English") {
	$"0006 0745 6E67 6C69 7368 0541 6772 6565"            /* ...English.Agree */
	$"0844 6973 6167 7265 6505 5072 696E 7407"            /* .Disagree.Print. */
	$"5361 7665 2E2E 2E7B 4966 2079 6F75 2061"            /* Save...{If you a */
	$"6772 6565 2077 6974 6820 7468 6520 7465"            /* gree with the te */
	$"726D 7320 6F66 2074 6869 7320 6C69 6365"            /* rms of this lice */
	$"6E73 652C 2070 7265 7373 2022 4167 7265"            /* nse, press "Agre */
	$"6522 2074 6F20 696E 7374 616C 6C20 7468"            /* e" to install th */
	$"6520 736F 6674 7761 7265 2E20 2049 6620"            /* e software.  If  */
	$"796F 7520 646F 206E 6F74 2061 6772 6565"            /* you do not agree */
	$"2C20 7072 6573 7320 2244 6973 6167 7265"            /* , press "Disagre */
	$"6522 2E"                                            /* e". */
};

data 'STR#' (5003, "Spanish") {
	$"0006 0745 7370 6196 6F6C 0741 6365 7074"            /* ...Español.Acept */
	$"6172 0A4E 6F20 6163 6570 7461 7208 496D"            /* ar¬No aceptar.Im */
	$"7072 696D 6972 0A47 7561 7264 6172 2E2E"            /* primir¬Guardar.. */
	$"2EC0 5369 2065 7374 8720 6465 2061 6375"            /* .¿Si está de acu */
	$"6572 646F 2063 6F6E 206C 6F73 2074 8E72"            /* erdo con los tér */
	$"6D69 6E6F 7320 6465 2065 7374 6120 6C69"            /* minos de esta li */
	$"6365 6E63 6961 2C20 7075 6C73 6520 2241"            /* cencia, pulse "A */
	$"6365 7074 6172 2220 7061 7261 2069 6E73"            /* ceptar" para ins */
	$"7461 6C61 7220 656C 2073 6F66 7477 6172"            /* talar el softwar */
	$"652E 2045 6E20 656C 2073 7570 7565 7374"            /* e. En el supuest */
	$"6F20 6465 2071 7565 206E 6F20 6573 748E"            /* o de que no esté */
	$"2064 6520 6163 7565 7264 6F20 636F 6E20"            /*  de acuerdo con  */
	$"6C6F 7320 748E 726D 696E 6F73 2064 6520"            /* los términos de  */
	$"6573 7461 206C 6963 656E 6369 612C 2070"            /* esta licencia, p */
	$"756C 7365 2022 4E6F 2061 6365 7074 6172"            /* ulse "No aceptar */
	$"2E22"                                               /* ." */
};

data 'STR#' (5004, "French") {
	$"0006 0846 7261 6E8D 6169 7308 4163 6365"            /* ...Français.Acce */
	$"7074 6572 0752 6566 7573 6572 0849 6D70"            /* pter.Refuser.Imp */
	$"7269 6D65 720E 456E 7265 6769 7374 7265"            /* rimer.Enregistre */
	$"722E 2E2E BA53 6920 766F 7573 2061 6363"            /* r...∫Si vous acc */
	$"6570 7465 7A20 6C65 7320 7465 726D 6573"            /* eptez les termes */
	$"2064 6520 6C61 2070 728E 7365 6E74 6520"            /*  de la présente  */
	$"6C69 6365 6E63 652C 2063 6C69 7175 657A"            /* licence, cliquez */
	$"2073 7572 2022 4163 6365 7074 6572 2220"            /*  sur "Accepter"  */
	$"6166 696E 2064 2769 6E73 7461 6C6C 6572"            /* afin d'installer */
	$"206C 6520 6C6F 6769 6369 656C 2E20 5369"            /*  le logiciel. Si */
	$"2076 6F75 7320 6E27 9074 6573 2070 6173"            /*  vous n'êtes pas */
	$"2064 2761 6363 6F72 6420 6176 6563 206C"            /*  d'accord avec l */
	$"6573 2074 6572 6D65 7320 6465 206C 6120"            /* es termes de la  */
	$"6C69 6365 6E63 652C 2063 6C69 7175 657A"            /* licence, cliquez */
	$"2073 7572 2022 5265 6675 7365 7222 2E"              /*  sur "Refuser". */
};

data 'STR#' (5005, "Italian") {
	$"0006 0849 7461 6C69 616E 6F07 4163 6365"            /* ...Italiano.Acce */
	$"7474 6F07 5269 6669 7574 6F06 5374 616D"            /* tto.Rifiuto.Stam */
	$"7061 0B52 6567 6973 7472 612E 2E2E 7F53"            /* pa.Registra....S */
	$"6520 6163 6365 7474 6920 6C65 2063 6F6E"            /* e accetti le con */
	$"6469 7A69 6F6E 6920 6469 2071 7565 7374"            /* dizioni di quest */
	$"6120 6C69 6365 6E7A 612C 2066 6169 2063"            /* a licenza, fai c */
	$"6C69 6320 7375 2022 4163 6365 7474 6F22"            /* lic su "Accetto" */
	$"2070 6572 2069 6E73 7461 6C6C 6172 6520"            /*  per installare  */
	$"696C 2073 6F66 7477 6172 652E 2041 6C74"            /* il software. Alt */
	$"7269 6D65 6E74 6920 6661 6920 636C 6963"            /* rimenti fai clic */
	$"2073 7520 2252 6966 6975 746F 222E"                 /*  su "Rifiuto". */
};

data 'STR#' (5006, "Japanese") {
	$"0006 084A 6170 616E 6573 650A 93AF 88D3"            /* ...Japanese¬ìØà” */
	$"82B5 82DC 82B7 0C93 AF88 D382 B582 DC82"            /* ÇµÇ‹Ç∑.ìØà”ÇµÇ‹Ç */
	$"B982 F108 88F3 8DFC 82B7 82E9 0795 DB91"            /* πÇÒ.àÛç¸Ç∑ÇÈ.ï€ë */
	$"B62E 2E2E B496 7B83 5C83 7483 6783 4583"            /* ∂...¥ñ{É\ÉtÉgÉEÉ */
	$"4783 418E 6797 708B 9691 F88C 5F96 F182"            /* GÉAégópãñë¯å_ñÒÇ */
	$"CC8F F08C 8F82 C993 AF88 D382 B382 EA82"            /* ÃèåèÇ…ìØà”Ç≥ÇÍÇ */
	$"E98F EA8D 8782 C982 CD81 4183 5C83 7483"            /* ÈèÍçáÇ…ÇÕÅAÉ\ÉtÉ */
	$"6783 4583 4783 4182 F083 4383 9383 5883"            /* gÉEÉGÉAÇÉCÉìÉXÉ */
	$"6781 5B83 8B82 B782 E982 BD82 DF82 C981"            /* gÅ[ÉãÇ∑ÇÈÇΩÇﬂÇ…Å */
	$"7593 AF88 D382 B582 DC82 B781 7682 F089"            /* uìØà”ÇµÇ‹Ç∑ÅvÇâ */
	$"9F82 B582 C482 AD82 BE82 B382 A281 4281"            /* üÇµÇƒÇ≠ÇæÇ≥Ç¢ÅBÅ */
	$"4093 AF88 D382 B382 EA82 C882 A28F EA8D"            /* @ìØà”Ç≥ÇÍÇ»Ç¢èÍç */
	$"8782 C982 CD81 4181 7593 AF88 D382 B582"            /* áÇ…ÇÕÅAÅuìØà”ÇµÇ */
	$"DC82 B982 F181 7682 F089 9F82 B582 C482"            /* ‹ÇπÇÒÅvÇâüÇµÇƒÇ */
	$"AD82 BE82 B382 A281 42"                             /* ≠ÇæÇ≥Ç¢ÅB */
};

data 'STR#' (5007, "Dutch") {
	$"0006 0A4E 6564 6572 6C61 6E64 7302 4A61"            /* ..¬Nederlands.Ja */
	$"034E 6565 0550 7269 6E74 0942 6577 6161"            /* .Nee.Print∆Bewaa */
	$"722E 2E2E A449 6E64 6965 6E20 7520 616B"            /* r...§Indien u ak */
	$"6B6F 6F72 6420 6761 6174 206D 6574 2064"            /* koord gaat met d */
	$"6520 766F 6F72 7761 6172 6465 6E20 7661"            /* e voorwaarden va */
	$"6E20 6465 7A65 206C 6963 656E 7469 652C"            /* n deze licentie, */
	$"206B 756E 7420 7520 6F70 2027 4A61 2720"            /*  kunt u op 'Ja'  */
	$"6B6C 696B 6B65 6E20 6F6D 2064 6520 7072"            /* klikken om de pr */
	$"6F67 7261 6D6D 6174 7575 7220 7465 2069"            /* ogrammatuur te i */
	$"6E73 7461 6C6C 6572 656E 2E20 496E 6469"            /* nstalleren. Indi */
	$"656E 2075 206E 6965 7420 616B 6B6F 6F72"            /* en u niet akkoor */
	$"6420 6761 6174 2C20 6B6C 696B 7420 7520"            /* d gaat, klikt u  */
	$"6F70 2027 4E65 6527 2E"                             /* op 'Nee'. */
};

data 'STR#' (5008, "Swedish") {
	$"0006 0653 7665 6E73 6B08 476F 646B 8A6E"            /* ...Svensk.Godkän */
	$"6E73 0641 7662 9A6A 7308 536B 7269 7620"            /* ns.Avböjs.Skriv  */
	$"7574 0853 7061 7261 2E2E 2E93 4F6D 2044"            /* ut.Spara...ìOm D */
	$"7520 676F 646B 8A6E 6E65 7220 6C69 6365"            /* u godkänner lice */
	$"6E73 7669 6C6C 6B6F 7265 6E20 6B6C 6963"            /* nsvillkoren klic */
	$"6B61 2070 8C20 2247 6F64 6B8A 6E6E 7322"            /* ka på "Godkänns" */
	$"2066 9A72 2061 7474 2069 6E73 7461 6C6C"            /*  för att install */
	$"6572 6120 7072 6F67 7261 6D70 726F 6475"            /* era programprodu */
	$"6B74 656E 2E20 4F6D 2044 7520 696E 7465"            /* kten. Om Du inte */
	$"2067 6F64 6B8A 6E6E 6572 206C 6963 656E"            /*  godkänner licen */
	$"7376 696C 6C6B 6F72 656E 2C20 6B6C 6963"            /* svillkoren, klic */
	$"6B61 2070 8C20 2241 7662 9A6A 7322 2E"              /* ka på "Avböjs". */
};

data 'STR#' (5009, "Brazilian Portuguese") {
	$"0006 1150 6F72 7475 6775 9073 2C20 4272"            /* ...Português, Br */
	$"6173 696C 0943 6F6E 636F 7264 6172 0944"            /* asil∆Concordar∆D */
	$"6973 636F 7264 6172 0849 6D70 7269 6D69"            /* iscordar.Imprimi */
	$"7209 5361 6C76 6172 2E2E 2E8C 5365 2065"            /* r∆Salvar...åSe e */
	$"7374 8720 6465 2061 636F 7264 6F20 636F"            /* stá de acordo co */
	$"6D20 6F73 2074 6572 6D6F 7320 6465 7374"            /* m os termos dest */
	$"6120 6C69 6365 6E8D 612C 2070 7265 7373"            /* a licença, press */
	$"696F 6E65 2022 436F 6E63 6F72 6461 7222"            /* ione "Concordar" */
	$"2070 6172 6120 696E 7374 616C 6172 206F"            /*  para instalar o */
	$"2073 6F66 7477 6172 652E 2053 6520 6E8B"            /*  software. Se nã */
	$"6F20 6573 7487 2064 6520 6163 6F72 646F"            /* o está de acordo */
	$"2C20 7072 6573 7369 6F6E 6520 2244 6973"            /* , pressione "Dis */
	$"636F 7264 6172 222E"                                /* cordar". */
};

data 'STR#' (5010, "Simplified Chinese") {
	$"0006 1253 696D 706C 6966 6965 6420 4368"            /* ...Simplified Ch */
	$"696E 6573 6504 CDAC D2E2 06B2 BBCD ACD2"            /* inese.Õ¨“‚.≤ªÕ¨“ */
	$"E204 B4F2 D3A1 06B4 E6B4 A2A1 AD54 C8E7"            /* ‚.¥Ú”°.¥Ê¥¢°≠T»Á */
	$"B9FB C4FA CDAC D2E2 B1BE D0ED BFC9 D0AD"            /* π˚ƒ˙Õ¨“‚±æ–Ìø…–≠ */
	$"D2E9 B5C4 CCF5 BFEE A3AC C7EB B0B4 A1B0"            /* “ÈµƒÃıøÓ£¨«Î∞¥°∞ */
	$"CDAC D2E2 A1B1 C0B4 B0B2 D7B0 B4CB C8ED"            /* Õ¨“‚°±¿¥∞≤◊∞¥À»Ì */
	$"BCFE A1A3 C8E7 B9FB C4FA B2BB CDAC D2E2"            /* º˛°£»Áπ˚ƒ˙≤ªÕ¨“‚ */
	$"A3AC C7EB B0B4 A1B0 B2BB CDAC D2E2 A1B1"            /* £¨«Î∞¥°∞≤ªÕ¨“‚°± */
	$"A1A3"                                               /* °£ */
};

data 'STR#' (5011, "Traditional Chinese") {
	$"0006 1354 7261 6469 7469 6F6E 616C 2043"            /* ...Traditional C */
	$"6869 6E65 7365 04A6 50B7 4E06 A4A3 A650"            /* hinese.¶P∑N.§£¶P */
	$"B74E 04A6 43A6 4C06 C078 A673 A14B 50A6"            /* ∑N.¶C¶L.¿x¶s°KP¶ */
	$"70AA 47B1 7AA6 50B7 4EA5 BBB3 5CA5 69C3"            /* p™G±z¶P∑N•ª≥\•i√ */
	$"D2B8 CCAA BAB1 F8B4 DAA1 41BD D0AB F6A1"            /* “∏Ã™∫±¯¥⁄°AΩ–´ˆ° */
	$"A7A6 50B7 4EA1 A8A5 48A6 77B8 CBB3 6EC5"            /* ß¶P∑N°®•H¶w∏À≥n≈ */
	$"E9A1 43A6 70AA 47A4 A3A6 50B7 4EA1 41BD"            /* È°C¶p™G§£¶P∑N°AΩ */
	$"D0AB F6A1 A7A4 A3A6 50B7 4EA1 A8A1 43"              /* –´ˆ°ß§£¶P∑N°®°C */
};

data 'STR#' (5012, "Danish") {
	$"0006 0544 616E 736B 0445 6E69 6705 5565"            /* ...Dansk.Enig.Ue */
	$"6E69 6707 5564 736B 7269 760A 4172 6B69"            /* nig.Udskriv¬Arki */
	$"7665 722E 2E2E 9848 7669 7320 6475 2061"            /* ver...òHvis du a */
	$"6363 6570 7465 7265 7220 6265 7469 6E67"            /* ccepterer beting */
	$"656C 7365 726E 6520 6920 6C69 6365 6E73"            /* elserne i licens */
	$"6166 7461 6C65 6E2C 2073 6B61 6C20 6475"            /* aftalen, skal du */
	$"206B 6C69 6B6B 6520 708C 20D2 456E 6967"            /*  klikke på “Enig */
	$"D320 666F 7220 6174 2069 6E73 7461 6C6C"            /* ” for at install */
	$"6572 6520 736F 6674 7761 7265 6E2E 204B"            /* ere softwaren. K */
	$"6C69 6B20 708C 20D2 5565 6E69 67D3 2066"            /* lik på “Uenig” f */
	$"6F72 2061 7420 616E 6E75 6C6C 6572 6520"            /* or at annullere  */
	$"696E 7374 616C 6C65 7269 6E67 656E 2E"              /* installeringen. */
};

data 'STR#' (5013, "Finnish") {
	$"0006 0553 756F 6D69 0848 7976 8A6B 7379"            /* ...Suomi.Hyväksy */
	$"6E0A 456E 2068 7976 8A6B 7379 0754 756C"            /* n¬En hyväksy.Tul */
	$"6F73 7461 0954 616C 6C65 6E6E 61C9 6F48"            /* osta∆Tallenna…oH */
	$"7976 8A6B 7379 206C 6973 656E 7373 6973"            /* yväksy lisenssis */
	$"6F70 696D 756B 7365 6E20 6568 646F 7420"            /* opimuksen ehdot  */
	$"6F73 6F69 7474 616D 616C 6C61 20D5 4879"            /* osoittamalla ’Hy */
	$"768A 6B73 79D5 2E20 4A6F 7320 6574 2068"            /* väksy’. Jos et h */
	$"7976 8A6B 7379 2073 6F70 696D 756B 7365"            /* yväksy sopimukse */
	$"6E20 6568 746F 6A61 2C20 6F73 6F69 7461"            /* n ehtoja, osoita */
	$"20D5 456E 2068 7976 8A6B 7379 D52E"                 /*  ’En hyväksy’. */
};

data 'STR#' (5014, "French Canadian") {
	$"0006 1146 7261 6E8D 6169 7320 6361 6E61"            /* ...Français cana */
	$"6469 656E 0841 6363 6570 7465 7207 5265"            /* dien.Accepter.Re */
	$"6675 7365 7208 496D 7072 696D 6572 0E45"            /* fuser.Imprimer.E */
	$"6E72 6567 6973 7472 6572 2E2E 2EBA 5369"            /* nregistrer...∫Si */
	$"2076 6F75 7320 6163 6365 7074 657A 206C"            /*  vous acceptez l */
	$"6573 2074 6572 6D65 7320 6465 206C 6120"            /* es termes de la  */
	$"7072 8E73 656E 7465 206C 6963 656E 6365"            /* présente licence */
	$"2C20 636C 6971 7565 7A20 7375 7220 2241"            /* , cliquez sur "A */
	$"6363 6570 7465 7222 2061 6669 6E20 6427"            /* ccepter" afin d' */
	$"696E 7374 616C 6C65 7220 6C65 206C 6F67"            /* installer le log */
	$"6963 6965 6C2E 2053 6920 766F 7573 206E"            /* iciel. Si vous n */
	$"2790 7465 7320 7061 7320 6427 6163 636F"            /* 'êtes pas d'acco */
	$"7264 2061 7665 6320 6C65 7320 7465 726D"            /* rd avec les term */
	$"6573 2064 6520 6C61 206C 6963 656E 6365"            /* es de la licence */
	$"2C20 636C 6971 7565 7A20 7375 7220 2252"            /* , cliquez sur "R */
	$"6566 7573 6572 222E"                                /* efuser". */
};

data 'STR#' (5015, "Korean") {
	$"0006 064B 6F72 6561 6E04 B5BF C0C7 09B5"            /* ...Korean.µø¿«∆µ */
	$"BFC0 C720 BEC8 C7D4 06C7 C1B8 B0C6 AE07"            /* ø¿« æ»«‘.«¡∏∞∆Æ. */
	$"C0FA C0E5 2E2E 2E7E BBE7 BFEB 20B0 E8BE"            /* ¿˙¿Â...~ªÁøÎ ∞Ëæ */
	$"E0BC ADC0 C720 B3BB BFEB BFA1 20B5 BFC0"            /* ‡º≠¿« ≥ªøÎø° µø¿ */
	$"C7C7 CFB8 E92C 2022 B5BF C0C7 2220 B4DC"            /* ««œ∏È, "µø¿«" ¥‹ */
	$"C3DF B8A6 20B4 ADB7 AF20 BCD2 C7C1 C6AE"            /* √ﬂ∏¶ ¥≠∑Ø º“«¡∆Æ */
	$"BFFE BEEE B8A6 20BC B3C4 A1C7 CFBD CABD"            /* ø˛æÓ∏¶ º≥ƒ°«œΩ Ω */
	$"C3BF C02E 20B5 BFC0 C7C7 CFC1 F620 BECA"            /* √ø¿. µø¿««œ¡ˆ æ  */
	$"B4C2 B4D9 B8E9 2C20 22B5 BFC0 C720 BEC8"            /* ¥¬¥Ÿ∏È, "µø¿« æ» */
	$"C7D4 2220 B4DC C3DF B8A6 20B4 A9B8 A3BD"            /* «‘" ¥‹√ﬂ∏¶ ¥©∏£Ω */
	$"CABD C3BF C02E"                                     /*  Ω√ø¿. */
};

data 'STR#' (5016, "Norwegian") {
	$"0006 054E 6F72 736B 0445 6E69 6709 496B"            /* ...Norsk.Enig∆Ik */
	$"6B65 2065 6E69 6708 536B 7269 7620 7574"            /* ke enig.Skriv ut */
	$"0A41 726B 6976 6572 2E2E 2EA3 4876 6973"            /* ¬Arkiver...£Hvis */
	$"2044 6520 6572 2065 6E69 6720 6920 6265"            /*  De er enig i be */
	$"7374 656D 6D65 6C73 656E 6520 6920 6465"            /* stemmelsene i de */
	$"6E6E 6520 6C69 7365 6E73 6176 7461 6C65"            /* nne lisensavtale */
	$"6E2C 206B 6C69 6B6B 6572 2044 6520 708C"            /* n, klikker De på */
	$"2022 456E 6967 222D 6B6E 6170 7065 6E20"            /*  "Enig"-knappen  */
	$"666F 7220 8C20 696E 7374 616C 6C65 7265"            /* for å installere */
	$"2070 726F 6772 616D 7661 7265 6E2E 2048"            /*  programvaren. H */
	$"7669 7320 4465 2069 6B6B 6520 6572 2065"            /* vis De ikke er e */
	$"6E69 672C 206B 6C69 6B6B 6572 2044 6520"            /* nig, klikker De  */
	$"708C 2022 496B 6B65 2065 6E69 6722 2E"              /* på "Ikke enig". */
};

data 'TEXT' (5000, "English SLA") {
	$"5468 6973 2069 7320 6120 7465 7374 206C"            /* This is a test l */
	$"6963 656E 7365 2061 6772 6565 6D65 6E74"            /* icense agreement */
	$"2028 616B 6120 2253 4C41 2229 210D 2869"            /*  (aka "SLA")!.(i */
	$"6420 3530 3030 29"                                  /* d 5000) */
};

data 'TEXT' (5004, "French SLA") {
	$"4327 6573 7420 756E 2063 6F6E 7472 6174"            /* C'est un contrat */
	$"2064 6520 6C69 6365 6E63 6520 6C6F 6769"            /*  de licence logi */
	$"6369 656C 2E0D 2869 6420 3530 3034 29"              /* ciel..(id 5004) */
};

data 'styl' (5000, "English SLA") {
	$"0003 0000 0000 000C 0009 0014 0000 0000"            /* .........∆...... */
	$"0000 0000 0000 0000 0027 000C 0009 0014"            /* .........'...∆.. */
	$"0100 0000 0000 0000 0000 0000 002A 000C"            /* .............*.. */
	$"0009 0014 0000 0000 0000 0000 0000"                 /* .∆............ */
};

data 'styl' (5004, "French SLA") {
	$"0003 0000 0000 000C 0009 0015 0000 0000"            /* .........∆...... */
	$"0000 0000 0000 0000 002A 000C 0009 0015"            /* .........*...∆.. */
	$"0100 0000 0000 0000 0000 0000 002E 000C"            /* ................ */
	$"0009 0015 0000 0000 0000 0000 0000"                 /* .∆............ */
};

