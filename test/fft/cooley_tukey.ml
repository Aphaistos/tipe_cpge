(* Structure pour manipuler les nombres complexes nécessaires à la FFT *)
type complex = { re : float; im : float }

(* Échange deux éléments dans un tableau *)
let swap arr i j =
  let temp = arr.(i) in
  arr.(i) <- arr.(j);
  arr.(j) <- temp

(* Permutation d'inversion des bits (Bit-Reversal) 
   Nécessaire pour l'algorithme in-place de Cooley-Tukey *)
let bit_reverse_permutation arr n =
  let j = ref 0 in
  for i = 0 to n - 2 do
    if i < !j then swap arr i !j;
    let k = ref (n / 2) in
    while !k <= !j do
      j := !j - !k;
      k := !k / 2
    done;
    j := !j + !k
  done

(* Algorithme de Cooley-Tukey FFT itératif et in-place *)
let fft_inplace arr =
  let n = Array.length arr in
  (* 1. Réordonner le tableau en inversant les bits *)
  bit_reverse_permutation arr n;
  
  (* 2. Boucles itératives de la FFT (Combinaison des "papillons") *)
  let len = ref 2 in
  while !len <= n do
    let half_len = !len / 2 in
    (* Calcul de la racine primitive de l'unité w_len = e^(-2iπ / len) *)
    let angle = -2.0 *. Float.pi /. (float_of_int !len) in
    let wlen_re = cos angle in
    let wlen_im = sin angle in
    
    let i = ref 0 in
    while !i < n do
      let w_re = ref 1.0 in
      let w_im = ref 0.0 in
      
      for j = 0 to half_len - 1 do
        let idx1 = !i + j in
        let idx2 = idx1 + half_len in
        
        (* Produit complexe : t = arr.(idx2) * w *)
        let t_re = arr.(idx2).re *. !w_re -. arr.(idx2).im *. !w_im in
        let t_im = arr.(idx2).re *. !w_im +. arr.(idx2).im *. !w_re in
        
        (* Mise à jour in-place (Structure Papillon) *)
        arr.(idx2) <- { re = arr.(idx1).re -. t_re; im = arr.(idx1).im -. t_im };
        arr.(idx1) <- { re = arr.(idx1).re +. t_re; im = arr.(idx1).im +. t_im };
        
        (* w = w * wlen *)
        let next_w_re = !w_re *. wlen_re -. !w_im *. wlen_im in
        let next_w_im = !w_re *. wlen_im +. !w_im *. wlen_re in
        w_re := next_w_re;
        w_im := next_w_im
      done;
      i := !i + !len
    done;
    len := !len * 2
  done

(* --- Fonctions d'Entrées/Sorties pour les fichiers audio Raw --- *)

(* Lit un fichier d'échantillons Raw Float64 et renvoie un tableau de complexes *)
let read_raw_audio filename =
  let ic = open_in_bin filename in
  let len = in_channel_length ic in
  let sample_count = len / 8 in (* 8 octets par float64 *)
  
  (* On s'assure que la taille est une puissance de 2 pour Cooley-Tukey *)
  let rec next_power_of_2 x =
    if x <= 1 then 1 else 2 * next_power_of_2 (x / 2) in
  let n = next_power_of_2 sample_count in
  
  let arr = Array.make n { re = 0.0; im = 0.0 } in
  try
    for i = 0 to sample_count - 1 do
      let bits = Int64.bits_of_float (Int64.of_int (0)) in (* Dummy init *)
      (* Lecture des 8 octets du float64 *)
      let buf = Bytes.create 8 in
      really_input ic buf 0 8;
      let int_val = ref 0L in
      for b = 0 to 7 do
        let byte = Int64.of_int (Char.code (Bytes.get buf b)) in
        int_val := Int64.logor !int_val (Int64.shift_left byte (b * 8))
      done;
      let float_val = Int64.float_of_bits !int_val in
      arr.(i) <- { re = float_val; im = 0.0 }
    done;
    close_in ic;
    arr
  with End_of_file ->
    close_in ic;
    arr

(* Écrit les coefficients complexes de la FFT dans un fichier binaire de sortie *)
let write_coefficients filename arr =
  let oc = open_out_bin filename in
  let n = Array.length arr in
  for i = 0 to n - 1 do
    (* Écriture de la partie réelle puis imaginaire (8 octets chacune) *)
    let write_float f =
      let bits = Int64.bits_of_float f in
      for b = 0 to 7 do
        let byte = Int64.to_int (Int64.logand (Int64.shift_right bits (b * 8)) 0xFFL) in
        output_byte oc byte
      done
    in
    write_float arr.(i).re;
    write_float arr.(i).im
  done;
  close_out oc

(* --- Programme Principal --- *)
let () =
  let input_file = "input.raw" in
  let output_file = "output_fft.raw" in
  
  print_endline "Lecture du fichier audio...";
  let audio_data = read_raw_audio input_file in
  let n = Array.length audio_data in
  Printf.printf "Taille du tableau traitée (puissance de 2) : %d\n" n;
  
  print_endline "Calcul de la FFT in-place (Cooley-Tukey)...";
  fft_inplace audio_data;
  
  print_endline "Écriture des coefficients...";
  write_coefficients output_file audio_data;
  print_endline "Traitement terminé avec succès."
