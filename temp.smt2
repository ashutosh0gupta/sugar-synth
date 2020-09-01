(declare-fun b_5_r_c () Bool)
(declare-fun b_4_r_e () Bool)
(declare-fun b_0_r_e () Bool)
(declare-fun b_1_r_c () Bool)
(declare-fun i_16_c () Int)
(declare-fun i_12_r_comp () Int)
(declare-fun b_9_r_c () Bool)
(declare-fun b_8_r_e () Bool)
(declare-fun b_19_s () Bool)
(declare-fun b_11_r_s () Bool)
(declare-fun b_18_s () Bool)
(declare-fun b_10_r_s () Bool)
(declare-fun b_31_s () Bool)
(declare-fun b_30_s () Bool)
(declare-fun i_22_c () Int)
(declare-fun b_25_s () Bool)
(declare-fun b_24_s () Bool)
(declare-fun b_55_s () Bool)
(declare-fun b_54_s () Bool)
(declare-fun i_40_c () Int)
(declare-fun b_43_s () Bool)
(declare-fun b_42_s () Bool)
(declare-fun i_34_c () Int)
(declare-fun b_37_s () Bool)
(declare-fun b_36_s () Bool)
(declare-fun b_49_s () Bool)
(declare-fun b_48_s () Bool)
(declare-fun b_17_rm () Bool)
(declare-fun b_29_rm () Bool)
(declare-fun i_28_c () Int)
(declare-fun b_23_rm () Bool)
(declare-fun b_47_rm () Bool)
(declare-fun b_53_rm () Bool)
(declare-fun i_52_c () Int)
(declare-fun b_44_cut () Bool)
(declare-fun b_26_cut () Bool)
(declare-fun b_50_cut () Bool)
(declare-fun i_39_ts () Int)
(declare-fun b_7_r_s () Bool)
(declare-fun b_6_r_s () Bool)
(declare-fun b_38_cut () Bool)
(declare-fun i_33_ts () Int)
(declare-fun b_3_r_s () Bool)
(declare-fun b_2_r_s () Bool)
(declare-fun i_46_c () Int)
(declare-fun i_45_ts () Int)
(declare-fun b_41_rm () Bool)
(declare-fun b_32_cut () Bool)
(declare-fun b_35_rm () Bool)
(declare-fun i_27_ts () Int)
(declare-fun i_51_ts () Int)
(declare-fun i_21_ts () Int)
(declare-fun b_20_cut () Bool)
(declare-fun i_15_ts () Int)
(declare-fun b_14_cut () Bool)
(declare-fun b_13_r_f () Bool)
(declare-fun b_68_g () Bool)
(assert (let ((a!1 (and (and (or (not b_6_r_s) b_48_s) (or (not b_7_r_s) b_49_s))
                (> i_27_ts i_45_ts)
                (>= i_12_r_comp i_46_c)))
      (a!2 (=> false
               (distinct (or b_1_r_c b_0_r_e) (=> (or b_30_s b_31_s) b_26_cut))))
      (a!3 (and (and (or (not b_2_r_s) b_30_s) (or (not b_3_r_s) b_31_s))
                (>= i_27_ts i_27_ts)
                (= i_28_c i_12_r_comp)))
      (a!5 (and (and (or (not b_6_r_s) b_24_s) (or (not b_7_r_s) b_25_s))
                (> i_15_ts i_21_ts)
                (>= i_12_r_comp i_22_c)))
      (a!6 (=> false
               (distinct (or b_1_r_c b_0_r_e) (=> (or b_18_s b_19_s) b_14_cut))))
      (a!7 (and (and (or (not b_2_r_s) b_18_s) (or (not b_3_r_s) b_19_s))
                (>= i_15_ts i_15_ts)
                (= i_16_c i_12_r_comp)))
      (a!9 (and (and (or (not b_2_r_s) b_18_s) (or (not b_3_r_s) b_19_s))
                (> i_21_ts i_15_ts)
                (>= i_12_r_comp i_16_c)))
      (a!10 (=> false
                (distinct (or b_5_r_c b_4_r_e) (=> (or b_24_s b_25_s) b_20_cut))))
      (a!11 (and (and (or (not b_6_r_s) b_24_s) (or (not b_7_r_s) b_25_s))
                 (>= i_21_ts i_21_ts)
                 (= i_22_c i_12_r_comp)))
      (a!13 (and (and (or (not b_2_r_s) b_30_s) (or (not b_3_r_s) b_31_s))
                 (> i_45_ts i_27_ts)
                 (>= i_12_r_comp i_28_c)))
      (a!14 (=> false
                (distinct (or b_5_r_c b_4_r_e) (=> (or b_48_s b_49_s) b_44_cut))))
      (a!15 (and (and (or (not b_6_r_s) b_48_s) (or (not b_7_r_s) b_49_s))
                 (>= i_45_ts i_45_ts)
                 (= i_46_c i_12_r_comp)))
      (a!17 (and (and (or (not b_6_r_s) b_42_s) (or (not b_7_r_s) b_43_s))
                 (> i_33_ts i_39_ts)
                 (>= i_12_r_comp i_40_c)))
      (a!18 (=> false
                (distinct (or b_1_r_c b_0_r_e) (=> (or b_36_s b_37_s) b_32_cut))))
      (a!19 (and (and (or (not b_2_r_s) b_36_s) (or (not b_3_r_s) b_37_s))
                 (>= i_33_ts i_33_ts)
                 (= i_34_c i_12_r_comp)))
      (a!21 (and (and (or (not b_2_r_s) b_36_s) (or (not b_3_r_s) b_37_s))
                 (> i_39_ts i_33_ts)
                 (>= i_12_r_comp i_34_c)))
      (a!22 (=> false
                (distinct (or b_5_r_c b_4_r_e) (=> (or b_42_s b_43_s) b_38_cut))))
      (a!23 (and (and (or (not b_6_r_s) b_42_s) (or (not b_7_r_s) b_43_s))
                 (>= i_39_ts i_39_ts)
                 (= i_40_c i_12_r_comp)))
      (a!25 (=> false
                (distinct (or false true) (=> (or b_54_s b_55_s) b_50_cut))))
      (a!26 (=> (or false true)
                (and (or (not true) b_54_s)
                     (or (not false) b_55_s)
                     true
                     (= i_52_c 0))))
      (a!28 (and (= b_53_rm b_29_rm)
                 (= b_54_s b_30_s)
                 (= b_55_s b_31_s)
                 true
                 (= b_47_rm b_23_rm)
                 (= b_48_s b_24_s)
                 (= b_49_s b_25_s)
                 false
                 false))
      (a!29 (and (= b_29_rm b_17_rm)
                 (= b_30_s b_18_s)
                 (= b_31_s b_19_s)
                 true
                 false))
      (a!32 (not (or false
                     false
                     false
                     false
                     false
                     false
                     false
                     false
                     false
                     false
                     false
                     false)))
      (a!33 (not (and (or (not b_10_r_s) b_18_s)
                      (or (not b_11_r_s) b_19_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_16_c)
                      (not (or b_5_r_c b_4_r_e))
                      b_0_r_e
                      (not b_1_r_c))))
      (a!34 (not (and (or (not b_10_r_s) b_18_s)
                      (or (not b_11_r_s) b_19_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_16_c)
                      (not (or b_1_r_c b_0_r_e))
                      b_4_r_e
                      (not b_5_r_c))))
      (a!35 (not (and (or (not b_10_r_s) b_24_s)
                      (or (not b_11_r_s) b_25_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_22_c)
                      (not (or b_5_r_c b_4_r_e))
                      b_0_r_e
                      (not b_1_r_c))))
      (a!36 (not (and (or (not b_10_r_s) b_24_s)
                      (or (not b_11_r_s) b_25_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_22_c)
                      (not (or b_1_r_c b_0_r_e))
                      b_4_r_e
                      (not b_5_r_c))))
      (a!37 (not (and (or (not b_10_r_s) b_36_s)
                      (or (not b_11_r_s) b_37_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_34_c)
                      (not (or b_5_r_c b_4_r_e))
                      b_0_r_e
                      (not b_1_r_c))))
      (a!38 (not (and (or (not b_10_r_s) b_36_s)
                      (or (not b_11_r_s) b_37_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_34_c)
                      (not (or b_1_r_c b_0_r_e))
                      b_4_r_e
                      (not b_5_r_c))))
      (a!39 (not (and (or (not b_10_r_s) b_42_s)
                      (or (not b_11_r_s) b_43_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_40_c)
                      (not (or b_5_r_c b_4_r_e))
                      b_0_r_e
                      (not b_1_r_c))))
      (a!40 (not (and (or (not b_10_r_s) b_42_s)
                      (or (not b_11_r_s) b_43_s)
                      b_8_r_e
                      b_9_r_c
                      (>= i_12_r_comp i_40_c)
                      (not (or b_1_r_c b_0_r_e))
                      b_4_r_e
                      (not b_5_r_c)))))
(let ((a!4 (and (or (not b_10_r_s) b_54_s)
                (or (not b_11_r_s) b_55_s)
                b_8_r_e
                b_9_r_c
                (> i_27_ts i_51_ts)
                (>= i_12_r_comp i_52_c)
                (=> (or b_5_r_c b_4_r_e) a!1)
                b_0_r_e
                (not b_1_r_c)
                a!2
                (=> (or b_1_r_c b_0_r_e) (=> (or b_18_s b_19_s) b_14_cut))
                (=> (or b_1_r_c b_0_r_e) (=> (or b_24_s b_25_s) b_20_cut))
                (=> (or b_1_r_c b_0_r_e) a!3)))
      (a!8 (or (and (or (not b_10_r_s) b_30_s)
                    (or (not b_11_r_s) b_31_s)
                    b_8_r_e
                    b_9_r_c
                    (> i_15_ts i_27_ts)
                    (>= i_12_r_comp i_28_c)
                    (=> (or b_5_r_c b_4_r_e) a!5)
                    b_0_r_e
                    (not b_1_r_c)
                    a!6
                    (=> (or b_1_r_c b_0_r_e) a!7))))
      (a!12 (or (and (or (not b_10_r_s) b_30_s)
                     (or (not b_11_r_s) b_31_s)
                     b_8_r_e
                     b_9_r_c
                     (> i_21_ts i_27_ts)
                     (>= i_12_r_comp i_28_c)
                     (=> (or b_1_r_c b_0_r_e) a!9)
                     b_4_r_e
                     (not b_5_r_c)
                     a!10
                     (=> (or b_5_r_c b_4_r_e) a!11))))
      (a!16 (and (or (not b_10_r_s) b_54_s)
                 (or (not b_11_r_s) b_55_s)
                 b_8_r_e
                 b_9_r_c
                 (> i_45_ts i_51_ts)
                 (>= i_12_r_comp i_52_c)
                 (=> (or b_1_r_c b_0_r_e) a!13)
                 b_4_r_e
                 (not b_5_r_c)
                 a!14
                 (=> (or b_5_r_c b_4_r_e) (=> (or b_36_s b_37_s) b_32_cut))
                 (=> (or b_5_r_c b_4_r_e) (=> (or b_42_s b_43_s) b_38_cut))
                 (=> (or b_5_r_c b_4_r_e) a!15)))
      (a!20 (or (and (or (not b_10_r_s) b_48_s)
                     (or (not b_11_r_s) b_49_s)
                     b_8_r_e
                     b_9_r_c
                     (> i_33_ts i_45_ts)
                     (>= i_12_r_comp i_46_c)
                     (=> (or b_5_r_c b_4_r_e) a!17)
                     b_0_r_e
                     (not b_1_r_c)
                     a!18
                     (=> (or b_1_r_c b_0_r_e) a!19))))
      (a!24 (or (and (or (not b_10_r_s) b_48_s)
                     (or (not b_11_r_s) b_49_s)
                     b_8_r_e
                     b_9_r_c
                     (> i_39_ts i_45_ts)
                     (>= i_12_r_comp i_46_c)
                     (=> (or b_1_r_c b_0_r_e) a!21)
                     b_4_r_e
                     (not b_5_r_c)
                     a!22
                     (=> (or b_5_r_c b_4_r_e) a!23))))
      (a!27 (and a!25
                 (=> (or false true) (=> (or b_30_s b_31_s) b_26_cut))
                 (=> (or false true) (=> (or b_48_s b_49_s) b_44_cut))
                 a!26))
      (a!30 (not (or and
                     (and (not b_30_s)
                          (= i_28_c i_52_c)
                          a!28
                          b_18_s
                          (not b_19_s)
                          false
                          true)
                     (and (not b_30_s)
                          (= i_28_c i_52_c)
                          a!28
                          (not b_18_s)
                          (= i_16_c i_28_c)
                          a!29
                          false)
                     false
                     and
                     false
                     false
                     false
                     and
                     false
                     false
                     false)))
      (a!31 (not (or and
                     (and (not b_18_s) (= i_16_c i_28_c) a!29 false)
                     false
                     false
                     and
                     false
                     false
                     false
                     and
                     false
                     false
                     false)))
      (a!41 (or (and (=> (<= i_16_c i_12_r_comp) (and a!33)))
                (and (=> (<= i_16_c i_12_r_comp) (and a!34)))
                (and (=> (<= i_22_c i_12_r_comp) (and a!35)))
                (and (=> (<= i_22_c i_12_r_comp) (and a!36)))
                (and (=> (<= i_34_c i_12_r_comp) (and a!37)))
                (and (=> (<= i_34_c i_12_r_comp) (and a!38)))
                (and (=> (<= i_40_c i_12_r_comp) (and a!39)))
                (and (=> (<= i_40_c i_12_r_comp) (and a!40)))))
      (a!42 (or (and (=> (<= i_16_c i_12_r_comp) (and a!33)))
                (and (=> (<= i_16_c i_12_r_comp) (and a!34)))
                (and (=> (<= i_22_c i_12_r_comp) (and a!35)))
                (and (=> (<= i_22_c i_12_r_comp) (and a!36)))))
      (a!43 (or (and (=> (<= i_16_c i_12_r_comp) (and a!33)))
                (and (=> (<= i_16_c i_12_r_comp) (and a!34))))))
(let ((a!44 (and (or (not b_54_s)
                     (and (not b_30_s) (or b_54_s b_55_s))
                     (and (not b_18_s) (or b_30_s b_31_s)))
                 (=> (not b_54_s) (not a!41))
                 (=> (and (not b_30_s) (or b_54_s b_55_s)) (not a!42))
                 (=> (and (not b_18_s) (or b_30_s b_31_s)) (not a!43)))))
(let ((a!45 (or (and (or b_55_s)
                     (not (or and
                              false
                              false
                              false
                              and
                              false
                              false
                              false
                              and
                              false
                              false
                              false)))
                (and (or b_31_s) a!30)
                (and (or b_19_s) a!31)
                (and (or b_24_s b_25_s) a!32)
                (and (or b_48_s b_49_s) a!32)
                a!44)))
  (and ((_ at-most 1) b_17_rm)
       (=> b_14_cut ((_ at-least 1) b_17_rm))
       ((_ at-most 1) b_23_rm)
       (=> b_20_cut ((_ at-least 1) b_23_rm))
       ((_ at-most 1) b_29_rm)
       (=> b_26_cut ((_ at-least 1) b_29_rm))
       ((_ at-most 1) b_35_rm)
       (=> b_32_cut ((_ at-least 1) b_35_rm))
       ((_ at-most 1) b_41_rm)
       (=> b_38_cut ((_ at-least 1) b_41_rm))
       ((_ at-most 1) b_47_rm)
       (=> b_44_cut ((_ at-least 1) b_47_rm))
       ((_ at-most 1) b_53_rm)
       (=> b_50_cut ((_ at-least 1) b_53_rm))
       ((_ at-most 1) b_18_s b_19_s)
       (=> (or b_18_s b_19_s) (or b_30_s b_31_s))
       ((_ at-most 1) b_24_s b_25_s)
       (=> (or b_24_s b_25_s) (or b_30_s b_31_s))
       ((_ at-most 1) b_30_s b_31_s)
       (=> (or b_30_s b_31_s) (or b_54_s b_55_s))
       ((_ at-most 1) b_36_s b_37_s)
       (=> (or b_36_s b_37_s) (or b_48_s b_49_s))
       ((_ at-most 1) b_42_s b_43_s)
       (=> (or b_42_s b_43_s) (or b_48_s b_49_s))
       ((_ at-most 1) b_48_s b_49_s)
       (=> (or b_48_s b_49_s) (or b_54_s b_55_s))
       ((_ at-most 1) b_54_s b_55_s)
       (or (or b_18_s b_19_s)
           (or b_24_s b_25_s)
           (or b_30_s b_31_s)
           (or b_36_s b_37_s)
           (or b_42_s b_43_s)
           (or b_48_s b_49_s))
       (=> (and b_50_cut b_53_rm) or)
       (=> (and b_26_cut b_29_rm) (or a!4))
       (=> (and b_14_cut b_17_rm) a!8)
       (=> (and b_20_cut b_23_rm) a!12)
       (=> (and b_44_cut b_47_rm) (or a!16))
       (=> (and b_32_cut b_35_rm) a!20)
       (=> (and b_38_cut b_41_rm) a!24)
       (or a!27)
       a!45))))))
(assert (=> b_68_g
    (and (= b_0_r_e true)
         (= b_1_r_c false)
         (= i_12_r_comp 0)
         (= b_13_r_f false)
         b_2_r_s
         (not b_3_r_s)
         (= b_4_r_e false)
         (= b_5_r_c false)
         (= i_12_r_comp 0)
         (= b_13_r_f false)
         (not b_6_r_s)
         b_7_r_s
         (= b_8_r_e true)
         (= b_9_r_c true)
         (= i_12_r_comp 0)
         (= b_13_r_f false)
         b_10_r_s
         (not b_11_r_s))))

