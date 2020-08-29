package com.cxytiandi.elementui.controller;

import com.cxytiandi.elementui.base.ResponseData;
import com.cxytiandi.elementui.vo.commentVo;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.List;

@RestController
@RequestMapping("/comment")
public class UserController {

	public List<commentVo> list = new ArrayList<>();

	@CrossOrigin(origins="*")
	@PostMapping("/save")
	public ResponseData save(@RequestBody commentVo comment) {
		list.add(comment);
		for(commentVo one : list){
			System.out.println("name="+one.getName()+"    context="+one.getContext());
		}

		return ResponseData.ok(list);
	}

	@GetMapping("/get")
	public ResponseData get() {

		return ResponseData.ok(list);


	}
}
