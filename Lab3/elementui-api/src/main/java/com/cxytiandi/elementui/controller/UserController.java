package com.cxytiandi.elementui.controller;

import com.cxytiandi.elementui.base.ResponseData;
import com.cxytiandi.elementui.vo.CommentVo;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.List;

@RestController
@RequestMapping("/comment")
public class UserController {

	public List<CommentVo> list = new ArrayList<>();

	@CrossOrigin(origins="*")
	@PostMapping("/save")
	public ResponseData save(@RequestBody CommentVo comment) {
		list.add(comment);
		for(CommentVo one : list){
			System.out.println("name="+one.getName()+"    context="+one.getContext());
		}

		return ResponseData.ok(list);
	}

	@GetMapping("/get")
	@CrossOrigin(origins="*")
	public ResponseData get() {

		return ResponseData.ok(list);


	}
}
